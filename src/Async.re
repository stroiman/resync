type t('a) = (('a => unit, exn => unit)) => unit;

let return = (x: 'a) : t('a) => ((cb, _)) => cb(x);

type duration = | MilliSeconds(int) | Seconds(int);
let toMilliseconds = fun | MilliSeconds(x) => x | Seconds(x) => x*1000;

exception JsError(Js.Exn.t);
exception Timeout;

let bind = (f: 'a => t('b), x: t('a)) : t('b) =>
  ((successCb, errorCB)) =>
    x((
      (a) =>
        try (f(a, (successCb, errorCB))) {
        | x => errorCB(x)
        },
      errorCB
    ));

let map = (f: 'a => 'b, x: t('a)) : t('b) =>
  ((successCb, errorCb)) =>
    x((
      (a) =>
        switch (f(a)) {
        | x => successCb(x)
        | exception x => errorCb(x)
        },
      errorCb
    ));

let tryCatch = (f: exn=>option('a), x:t('a)) : t('a) => {
  ((successCb, errorCb)) => x((successCb, 
    x => switch(f(x)) {
      | Some(x) => successCb(x)
      | None => errorCb(x)
      }));
};

let iter = f => map(x => {f(x); x});

let from_js = (jsAsync: ((Js.Null.t(Js.Exn.t), 'a) => unit) => unit) : t('a) =>
  ((successCb, errorCb)) =>
    jsAsync(
      (err, s) =>
        switch (Js.Null.toOption(err)) {
        | None => successCb(s)
        | Some(x) => errorCb(JsError(x))
        }
    );

let once = (x:t('a)) : t('a) => {
  let resolution = ref(None);
  let waiting = ref([]);

  let dispatch = (result, (cb, ecb)) => {
    switch(result) {
      | Js.Result.Ok(x) => cb(x);
      | Js.Result.Error(x) => ecb(x);
    }
  };

  let tryDispatch = () => 
      switch (resolution^) {
        | None => ()
        | Some(x) => { 
          let targets = waiting^;
          waiting := [];
          targets |> List.iter((callbacks) => dispatch(x,callbacks));
        };
      };

  let tryDispatch = () => Js.Global.setTimeout(tryDispatch, 0) |> ignore;
  x((s => { resolution:=Some(Js.Result.Ok(s)); tryDispatch() },
     e => { resolution:=Some(Js.Result.Error(e)); tryDispatch() }));

  (callbacks) => {
    switch(resolution^) {
      | None => waiting := [callbacks, ...(waiting^)]
      | Some(x) => dispatch(x,callbacks)
    }
  };
};

let timeout = (duration:duration, x:t('a)) : t('a) => {
  let resolved = ref (false);
  ((successCb, errorCb)) => {
    x((x => {
      if (!resolved^) {
        resolved := true;
        successCb(x);
      }
    }, e => {
      if (!resolved^) {
        resolved := true;
        errorCb(e);
      }
    }));
    Js.Global.setTimeout(() => {
      if (!resolved^) {
        resolved := true;
        errorCb(Timeout);
      }
    }, toMilliseconds(duration)) |> ignore;
  }
};

let from_callback = (fn: ('a => unit) => unit) : t('a) =>
  ((successCb, _)) => fn(successCb);

let run = (~fe=?, f : 'a => unit, x : t('a)) => {
  let fe : (exn => unit) =fe |> Js.Option.getWithDefault(ignore);
  x((f, fe));
};
 
let runExn = (~fs, ~fe, x) => {
  Js.log("OBSOLETE FUNCTION CALL, use Async.run insteadn of Async.runExn");
  x((fs, fe));
}
