/*
 * 4coder_system_types.h - Implementation of universal (cross platform) helpers
 */

// TOP

Mutex_Lock::Mutex_Lock(System_Mutex m){
    system_mutex_acquire(m);
    this->mutex = m;
}

Mutex_Lock::~Mutex_Lock(){
    system_mutex_release(this->mutex);
}

Mutex_Lock::operator System_Mutex(){
    return(this->mutex);
}

// BOTTOM

