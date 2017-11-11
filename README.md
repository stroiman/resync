# Reason module for Async

This module started its life because I was connecting to the js mongo driver
from reason code. The driver is asynchronous, and can be used with either
callbacks or promises. But using promises in the heart of my reason code just
felt wrong, so slowly work for this library began.

This library has been published as 're-sync' to npm

    npm install --save re-sync

or

    yarn add re-sync

## Description

The heart of the module is the type:

```
type t('a) = (('a => unit, exn => unit)) => unit;
```

So, it's a function that takes two callbacks, one that will be executed when
things succeed, and one that is executed when an exception is thrown.

So if you have such a function, you can use this library to glue functions
together that operate asynchronously.

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
 * `from_js` Helps creating an `Async.t` from an async javascript function. See
     exmaple later

Look at the example tests for a hint as to their usage.

## Only use the exception path for truly exceptional cases.

It is a common pattern to use a result type, like this.
```
type result('a,'b) =
  | Ok('a)
  | Error('b)
```

This library does not attempt to replace this pattern,
This type can still be used with the async module:
```
type asyncResult('a,'b) = async(result('a,'b));
```

This is used a lot in my own code, and exceptions are only used to handle truly
exceptional cases, that will result in HTTP 500 errors - e.g. broken database
connections, etc. Any error that can be handled in the application layer is
represented with the `Error()` constructor.

## Using with async JavaScript modules

A common pattern in JavaScript is to have a function accept a callback that
accepts two arguments, an error and a result. The error argument will receive
`null` if the operation succeeded.

`Resync` supports handling this case easily. This small adaption of the
`bsyncjs` module shows how:

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
    |> Async.from_js |> Async.map(~f=Js.to_bool);
}
```


