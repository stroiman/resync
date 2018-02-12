# Reason module for Async

Helper for async Reason/Bucklescript code without Promises.

[![Build Status](https://travis-ci.org/stroiman/resync.svg?branch=master)](https://travis-ci.org/stroiman/resync)

I wrote this library because I needed to deal with async code, but I wanted to
avoid using promises in order to keep the core of my application javascript
agnostic.

## Installation

Run `npm install --save @stroiman/async` and add `@stroiman/async` to the `bs-dependencies` in `bsconfig.json`.

## Documentation

The heart of this module is the type:

Reason syntax:
```Reason
module Async = {
  type t('a) = (('a => unit, exn => unit)) => unit;
}
```

OCaml syntax:
```ocaml
type 'a t = (('a -> unit) * (exn -> unit)) -> unit
```

So, it's a function that takes two callbacks, one that will be executed if an
operation succeeds, and one that will be executed if an exception is thrown.

So if you have such a function, you can use this library to glue functions
together that operate asynchronously.

If the last argument of your async function conforms to this signature, then you
can use currying to compose functions together with this library.

Useful funcitons.

 * `bind` Takes a function that returns an async result and use it in an existing
     async context.
 * `map` Takes a function that returns a sync result, and use it in an async
     context.
 * `return` Takes a value, and returns an async context resolving that value. In
     a promise context, this would correspond to `Promise.resolve`
 * `tryCatch` Takes a function that might handle an exception. Return `Some` if the
     exception was handled, and `None` if it wasn't.
 * `timeout` Helps handling handling functions that take too long to execute
 * `run(~fe=?,f)` Takes a callback to be called with the final value, and an
     optional callback to be called with any exceptions caught during execution.
 * `from_js` Helps creating an `Async.t` from an async javascript function. See
     exmaple later
 * `once` Takes something that resolves asynchronously, and allow it to be
     called multiple times, each time yielding the same result. A database
     connection pool factory is a good candidate.

Be aware that this library does not evaluate any values in advance. Nothing is
evaluated until you call the `run` function.

Look at the example tests for a hint as to their usage.

## Only use the exception path for truly exceptional cases.

It is a common pattern to use a result type, like this (defined in Js.Result).

```reason
type result('a,'b) =
  | Ok('a)
  | Error('b)
```

This library does not attempt to replace this pattern,
This type can still be used with the async module:

```reason
type asyncResult('a,'b) = Async.t(result('a,'b));
```

This is used a lot in my own code, and exceptions are only used to handle truly
exceptional cases, that will result in HTTP 500 errors - e.g. broken database
connections, etc. Any error that can be handled in the application layer is
represented with the `Error()` constructor.

## Using with async JavaScript modules

A common pattern in JavaScript is to have a function accept a callback that
accepts two arguments, an error and a result. The error argument will receive
`null` if the operation succeeded.

This library supports handling this case easily. This small adaption of the
`bcryptjs` module shows how:

```
module Bcrypt = {
  type error = Js.Exn.t;
  [@bs.module "bcryptjs"]
  external hash : (string, int, (Js.null(error), string) => unit) => unit = "";
  [@bs.module "bcryptjs"]
  external compare : (string, string, (Js.null(error), Js.boolean) => unit) => unit = "";

  let hash = (password, gen) => hash(password,gen)
    |> Async.from_js;
  let compare = (password, hash) => compare(password, hash)
    |> Async.from_js |> Async.map(Js.to_bool);
}
```


