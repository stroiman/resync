open Respect.Dsl.Async;;
open Async.Infix;;

describe "infix operators" [
  describe ">>=" [
    it "produces binds" (fun _ -> 
      let add a b = Async.return(a + b) in
      (Async.return 1
      >>= add 2)
      |> shoulda (asyncResolve >=> equal 3)
    )
  ]
] |> register;


