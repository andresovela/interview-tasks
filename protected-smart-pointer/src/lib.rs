use std::collections::HashSet;
use std::marker::PhantomData;
use std::ops::{Deref, DerefMut};
use std::sync::Arc;
use std::sync::RwLock;
use std::sync::RwLockReadGuard;
use std::sync::RwLockWriteGuard;

/// Zero-sized type used to mark instances of `Protected<T>` that
/// "own" the `T` in the sense that they manage access to it.
pub struct Owner;

/// Zero-sized type used to mark instances of `Protected<T>` that
/// may have access to `T` as long as the owner allows it.
pub struct User;

/// Indicates that the user no longer has access to `T`.
#[derive(Debug)]
pub struct AccessDeniedError;

/// RAII structure used to release the shared read access of a lock when dropped.
pub struct ProtectedReadGuard<'a, T>(RwLockReadGuard<'a, ProtectedBox<T>>);

/// RAII structure used to release the exclusive write access of a lock when dropped.
pub struct ProtectedWriteGuard<'a, T>(RwLockWriteGuard<'a, ProtectedBox<T>>);

/// A smart pointer that grants access to `T` for as long as the owner allows.
///
/// The owner of `T` is allowed to create/remove users that have access to `T`.
pub struct Protected<T, Access> {
    inner: Arc<RwLock<ProtectedBox<T>>>,
    access_key: Option<u32>,
    _marker: PhantomData<Access>,
}

/// Inner type of `Protected<T>`.
struct ProtectedBox<T> {
    value: T,
    access_keys: HashSet<u32>,
}

impl<T> Protected<T, Owner> {
    /// Creates a `Protected` access to `T`.
    ///
    /// The instance returned by this function is considered the _owner_ of `T`.
    pub fn new(value: T) -> Protected<T, Owner> {
        let inner = Arc::new(RwLock::new(ProtectedBox {
            value,
            access_keys: HashSet::new(),
        }));

        Protected {
            inner,
            access_key: None,
            _marker: PhantomData,
        }
    }

    /// Grants access to `T` to a user with a given ID.
    ///
    /// This function returns a new `Protected` access to `T`, only if
    /// a user with the given ID does not already exist.
    pub fn create_user(&self, id: u32) -> Option<Protected<T, User>> {
        let mut inner = self.inner.write().unwrap();
        let access_keys = &mut inner.access_keys;
        if access_keys.insert(id) {
            Some(Protected {
                inner: self.inner.clone(),
                access_key: Some(id),
                _marker: PhantomData,
            })
        } else {
            None
        }
    }

    /// Revokes access to `T` for a user with a given ID.
    pub fn remove_user(&self, id: u32) {
        let mut inner = self.inner.write().unwrap();
        let access_keys = &mut inner.access_keys;
        access_keys.remove(&id);
    }

    /// Locks this `T` so that the owner has shared read access to `T`.
    ///
    /// # Panics
    ///
    /// Under the hood, `read` uses a [`std::sync::RwLock`], and this function panics
    /// if the `RwLock` ever becomes poisoned.
    pub fn read(&self) -> ProtectedReadGuard<T> {
        ProtectedReadGuard(self.inner.read().unwrap())
    }

    /// Locks this `T` so that the owner has exclusive write access to `T`.
    ///
    /// # Panics
    ///
    /// Under the hood, `write` uses a [`std::sync::RwLock`], and this function panics
    /// if the `RwLock` ever becomes poisoned.
    pub fn write(&self) -> ProtectedWriteGuard<T> {
        ProtectedWriteGuard(self.inner.write().unwrap())
    }
}

impl<T> Protected<T, User> {
    /// Locks this `T` so that this user has shared read access to `T`.
    ///
    /// # Errors
    ///
    /// This function will return an error if the owner of `T` has been dropped,
    /// or if the owner has revoked this user from accessing `T`.
    ///
    /// # Panics
    ///
    /// Under the hood, `read` uses a [`std::sync::RwLock`], and this function panics
    /// if the `RwLock` ever becomes poisoned.
    pub fn read(&self) -> Result<ProtectedReadGuard<T>, AccessDeniedError> {
        if self.has_access() {
            Ok(ProtectedReadGuard(self.inner.read().unwrap()))
        } else {
            Err(AccessDeniedError)
        }
    }

    /// Locks this `T` so that this user has exclusive write access to `T`.
    ///
    /// # Errors
    ///
    /// This function will return an error if the owner of `T` has been dropped,
    /// or if the owner has revoked this user from accessing `T`.
    ///
    /// # Panics
    ///
    /// Under the hood, `write` uses a [`std::sync::RwLock`], and this function panics
    /// if the `RwLock` ever becomes poisoned.
    pub fn write(&self) -> Result<ProtectedWriteGuard<T>, AccessDeniedError> {
        if self.has_access() {
            Ok(ProtectedWriteGuard(self.inner.write().unwrap()))
        } else {
            Err(AccessDeniedError)
        }
    }

    /// Checks if this instance of Protected has access to `T`.
    ///
    /// A user only has access to `T` if its access key is found in
    /// the access keys for the `Protected<T>`.
    fn has_access(&self) -> bool {
        let inner = self.inner.read().unwrap();
        let access_keys = &inner.access_keys;
        access_keys.contains(&self.access_key.unwrap())
    }
}

impl<T, A> Drop for Protected<T, A> {
    fn drop(&mut self) {
        let mut inner = self.inner.write().unwrap();
        let access_keys = &mut inner.access_keys;
        if let Some(access_key) = self.access_key {
            // If this is a user of `T`, the user should resign to its own access
            // to T.
            access_keys.remove(&access_key);
        } else {
            // If the access key is None, then this is the owner of `T` and
            // all accesses to `T` should be revoked when the owner is dropped.
            access_keys.clear();
        }
    }
}

impl<'a, T> Deref for ProtectedReadGuard<'a, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        &self.0.value
    }
}

impl<'a, T> Deref for ProtectedWriteGuard<'a, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        &self.0.value
    }
}

impl<'a, T> DerefMut for ProtectedWriteGuard<'a, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0.value
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn owner_can_read() {
        let p = Protected::new(42);
        let x = p.read();
        assert_eq!(*x, 42);
    }

    #[test]
    fn owner_can_write() {
        let p = Protected::new(42);

        {
            let mut x = p.write();
            *x = 43;
        }

        let x = p.read();
        assert_eq!(*x, 43);
    }

    #[test]
    fn owner_cannot_create_duplicated_users() {
        let owner = Protected::new(42);
        let user1 = owner.create_user(0);
        let user2 = owner.create_user(0);
        assert!(user1.is_some());
        assert!(user2.is_none());
    }

    #[test]
    fn owner_can_create_user_with_previously_dropped_id() {
        let owner = Protected::new(42);
        let user1 = owner.create_user(0);
        assert!(user1.is_some());
        drop(user1);
        let user2 = owner.create_user(0);
        assert!(user2.is_some());
    }
 
    #[test]
    fn user_with_access_can_read() {
        let owner = Protected::new(42);
        let user = owner.create_user(0).unwrap();
        let x = user.read().unwrap();
        assert_eq!(*x, 42);
    }

    #[test]
    fn user_with_revoked_access_cannot_read() {
        let owner = Protected::new(42);
        let user = owner.create_user(0).unwrap();
        owner.remove_user(0);
        assert!(user.read().is_err())
    }

    #[test]
    fn user_without_access_cannot_read() {
        let owner = Protected::new(42);
        let user = owner.create_user(0).unwrap();
        drop(owner);
        assert!(user.read().is_err())
    }

    #[test]
    fn user_with_access_can_write() {
        let owner = Protected::new(42);
        let user = owner.create_user(0).unwrap();
        {
            let mut x = user.write().unwrap();
            *x = 43;
        }
        let x = user.read().unwrap();
        assert_eq!(*x, 43);
    }

    #[test]
    fn user_with_revoked_access_cannot_write() {
        let owner = Protected::new(42);
        let user = owner.create_user(0).unwrap();
        owner.remove_user(0);
        assert!(user.write().is_err())
    }

    #[test]
    fn user_without_access_cannot_write() {
        let owner = Protected::new(42);
        let user = owner.create_user(0).unwrap();
        drop(owner);
        assert!(user.write().is_err())
    }

    #[test]
    fn user_can_read_something_written_by_another_user() {
        let owner = Protected::new(42);
        let user1 = owner.create_user(0).unwrap();
        let user2 = owner.create_user(1).unwrap();
        {
            let mut x = user1.write().unwrap();
            *x = 43;
        }
        let x = user2.read().unwrap();
        assert_eq!(*x, 43);
    }
}
