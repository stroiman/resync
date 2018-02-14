open Respect.Matcher;

let haveMessage = expected : matcher(exn, option(string)) => actual => 
  switch(actual) {
    | Async.JsError(err) => equal(expected, Js.Exn.message(err))
    | _ => matchFailure(actual,expected)
    };

let asyncResolve : matcher(Async.t('a),'b) = (actual:Async.t('a)) => (cb : matchResult('b) => unit) => {
  let successCb = x => cb(MatchSuccess(x));
  let exnCb = x => cb(MatchFailure(x |> Obj.repr, x |> Obj.repr));
  actual |> Async.run(~fe=exnCb, successCb)
};

let asyncThrow : matcher(Async.t('a), exn) = (actual:Async.t('a)) => {
  let result = cb => {
    let successCb = x => cb(MatchFailure(x |> Obj.repr, x |> Obj.repr));
    let exnCb = x => cb(MatchSuccess(x));
    actual |> Async.run(successCb,~fe=exnCb)
    };
  result
};


