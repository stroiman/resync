open Respect.Dsl.Async;;
open Respect.Matcher;;
open Matchers;;

describe "Async.all" [
  it "resolves when all items resolve" (fun _ ->
    let a = (fun (cb,_) -> Js.Global.setTimeout(fun _ -> cb 1) 1 |> ignore) in
    let b = (fun (cb,_) -> Js.Global.setTimeout(fun _ -> cb 2) 3 |> ignore) in
    let c = (fun (cb,_) -> Js.Global.setTimeout(fun _ -> cb 3) 2 |> ignore) in
    [a; b; c] 
    |> Async.all
    |> shoulda (asyncResolve >=> equal [1; 2; 3])
  )
] |> register;
