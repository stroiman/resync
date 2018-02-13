open Respect.Dsl.Async;;
open Respect.Matcher;;
open Async.Infix;;
open Matchers;;

describe "infix operators" [
  describe ">>=" [
    it "produces binds" (fun _ -> 
      let add a b = Async.return(a + b) in
      let result = Async.return 1 >>= add 2 in
      result |> shoulda (asyncResolve >=> equal 3)
    )
  ]
] |> register;


