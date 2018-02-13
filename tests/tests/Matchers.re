open Respect.Matcher;

let asyncResolve : matcher(Async.t('a),'b) = (actual:Async.t('a)) => (cb : matchResult('b) => unit) => {
  let successCb = x => cb(MatchSuccess(x));
  let exnCb = x => cb(MatchFailure(x |> Obj.repr, x |> Obj.repr));
  actual |> Async.run(~fe=exnCb, successCb)
};
