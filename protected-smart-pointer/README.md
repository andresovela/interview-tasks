# Protected access control

The goal of this task was to implement a working mock-up of a `Protected<T>` struct in Rust.

`Protected<T>` should be a smart pointer that provides the means to control access to `T`.

The requirements were as follows:
- `Protected<T>` should be `Send` and `Sync`
- The owner should be able to track the users of `T`
- The inner `T` should only be accessible if the user has access to it
- Only the owner can add and remove users
- When a user terminates, the user automatically drops its access to `T`
- When the owner terminates, the owner revokes access to `T` for all users