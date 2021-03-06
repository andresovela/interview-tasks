use std::marker::PhantomData;

/// A hypothetical CAN interface.
///
/// # Example
///
/// ```
/// use can_interface::Can;
///
/// let can = Can::new();
/// let mut can = can.open();
///
/// let _ = can.send(42);
///
/// let mut can = can.set_receive();
/// let _ = can.blocking_receive().unwrap();
/// let _ = can.close();
/// ```
pub struct Can<State> {
    _marker: PhantomData<State>,
}

/// Open interface.
pub struct Open;

/// Interface in receiving mode.
pub struct Receiving;

/// Closed interface.
pub struct Closed;

impl Can<Closed> {
    /// Create a CAN interface.
    pub fn new() -> Self {
        Can {
            _marker: PhantomData,
        }
    }

    /// Open a closed interface.
    pub fn open(self) -> Can<Open> {
        Can {
            _marker: PhantomData,
        }
    }
}

impl Default for Can<Closed> {
    fn default() -> Self {
        Self::new()
    }
}

impl Can<Open> {
    /// Send a payload to an open interface.
    pub fn send(&mut self, payload: u32) -> Result<(), &str> {
        let _ = payload;
        Ok(())
    }

    /// Set the interface in receiving mode.
    pub fn set_receive(self) -> Can<Receiving> {
        Can {
            _marker: PhantomData,
        }
    }

    /// Close an open interface.
    pub fn close(self) -> Can<Closed> {
        Can {
            _marker: PhantomData,
        }
    }
}

impl Can<Receiving> {
    /// Block on an interface able to receive.
    pub fn blocking_receive(&mut self) -> Result<u32, &str> {
        Ok(100)
    }

    /// Close an open interface.
    pub fn close(self) -> Can<Closed> {
        Can {
            _marker: PhantomData,
        }
    }
}
