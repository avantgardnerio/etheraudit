use std;
use std::sync::RwLock;

type PtrType<T> = std::sync::Arc<T>;
type WeakPtrType<T> = std::sync::Weak<T>;

pub struct WeakPtr<T> {
    val: WeakPtrType<RwLock<T>>
}

impl<T> WeakPtr<T> {
    pub fn upgrade(&self) -> Option<SharedPtr<T>> {
        self.val.upgrade().map(|val| SharedPtr{val})
    }
}

pub struct SharedPtr<T> {
    val: PtrType<RwLock<T>>
}

impl<T> SharedPtr<T> {
    pub fn new(val: T) -> Self {
        Self { val: PtrType::new(RwLock::new(val)) }
    }
    pub fn downgrade(&self) -> WeakPtr<T> {
        WeakPtr { val: PtrType::downgrade(&self.val) }
    }

    pub fn get<'a>(&'a self) -> std::sync::RwLockReadGuard<'a, T> {
        self.val.read().unwrap()
    }

    pub fn get_mut<'a>(&'a self) -> std::sync::RwLockWriteGuard<'a, T> {
        self.val.write().unwrap()
    }
}

impl<T> Clone for SharedPtr<T> {
    fn clone(&self) -> SharedPtr<T> {
        Self { val: self.val.clone() }
    }
}
