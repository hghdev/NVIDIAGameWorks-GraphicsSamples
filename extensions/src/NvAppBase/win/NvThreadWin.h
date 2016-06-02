//----------------------------------------------------------------------------------
// File:        NvAppBase\win/NvThreadWin.h
// SDK Version: v3.00 
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014-2015, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------

#ifndef NV_THREADWIN_H
#define NV_THREADWIN_H

#include "NvAppBase\NvThread.h"

#include <Windows.h>
#include <SynchAPI.h>
#include <unordered_map>

/// \file NvThreadWin.h
/// Windows implementation of the threading wrapper API.

/// \brief Windows implementation of a thread instance.
/// It makes use of the Windows threads, but not of Windows' User Mode
/// Scheduling (UMS) as it is not needed.
/// https://msdn.microsoft.com/en-us/library/windows/desktop/ms684841(v=vs.85).aspx
class NvThreadWin : public NvThread {

public:

	/// \brief Default constructor
	/// Do NOT use. Overwritten for safety; it should throw an exception.
	NvThreadWin(void);

	/// \brief Copy constructor
	/// Do NOT use. Overwritten for safety; it should throw an exception.
	/// \param[in] obj reference to instance in attempted copy
	NvThreadWin(const NvThread &obj);

	/// \brief Constructor
	/// Only constructor that creates an instance of a thread. When created,
	/// execution will not start immediately (#startThread needs to be called
	/// first).
	/// \param[in] function the function pointer the thread should execute.
	/// \param[in] argument data structure with arguments to the function.
	/// \param[in] stack pointer to allocated stack memory for the thread.
	/// \param[in] stackSize size of allocated stack memory for the thread.
	///						 It must be aligned with the NVTHREAD_STACK_ALIGN
	///						 attribute declaration.
	/// \param[in] priority priority assigned to the thread; integer value
	///						between #HighestThreadPriority and
	///						#LowestThreadPriority.
	NvThreadWin(NvThreadFunction function, void* argument, void* stack,
		size_t stackSize, int priority);

	/// \brief Destructor
	virtual ~NvThreadWin(void);

	/// \name Threading API pure virtual methods
	/// To be overridden with platform specific calls by the implementing
	/// derived classes.
	///@{

	/// \brief Start thread execution.
	/// Starts execution of the assigned function for the subject thread.
	void startThread(void);

	/// \brief Wait for thread completion.
	/// Blocks the currently executing thread until the subject thread finishes
	/// execution of its assigned function.
	void waitThread(void);

	/// \brief Changes the priority of the thread.
	/// Changes the priority of the subject thread.
	/// \param[in] priority priority assigned to the thread; integer value
	///						between #HighestThreadPriority and
	///						#LowestThreadPriority.
	/// \return the previous priority value before the change.
	int changeThreadPriority(int priority);

	/// \brief Gets the original priority of the thread.
	/// Gets the priority of the subject thread as assigned at its creation
	/// (constructor call).
	/// \return the original priority value.
	int getThreadPriority(void) const;

	/// \brief Gets the current priority of the thread.
	/// Gets the current priority of the subject thread as determined by the
	/// last call to #changeThreadPriority, if any; otherwise, as determined
	/// by the constructor call.
	/// \return the current priority value.
	int getThreadCurrentPriority(void) const;

	/// \brief Sets the name of the thread.
	/// Sets the name of the thread; useful for debugging purposes. The pointer
	/// to the character string DOES NOT need to be kept for the thread to use.
	/// \param[in] name character string with the name.
	void setThreadName(const char* name);

	/// \brief Sets the name of the thread.
	/// Sets the name of the thread; useful for debugging purposes. The pointer
	/// to the character string DOES need to be kept for the thread to use.
	/// \param[in] name character string with the name.
	void setThreadNamePointer(const char* name);

	/// \brief Gets the name of the thread.
	/// Gets a pointer to character string with the name of the thread; useful
	/// for debugging purposes. Do NOT modify the string.
	/// \return pointer to the character string with the name.
	const char* getThreadNamePointer(void) const;

	///@}

	/// \name Thread accessor methods
	/// Used mostly by the #NvThreadManager instance to catalog and index
	/// threads
	///@{

	/// \brief Get thread ID
	/// \return the integer ID associated with this thread
	const DWORD getThreadId(void) const { return mThreadId; }

	///@}

private:

	/// \name Private helper methods
	///@{

	/// \brief Helper method that helps set a thread's name
	/// \param[in] name null-terminated string with the name
	void setThreadNamePrivate(const char* name);

	/// \brief Helper method that frees the thread name
	void freeThreadName(void);

	/// \brief Helper method that gets the priority of the thread
	/// \return integer representing the thread's priority
	int getThreadPriorityPrivate(void) const;

	/// \brief Helper method that maps NN priorities to Windows priorities.
	/// It maps from the integer priority range (according to the nn::os API)
	/// to the corresponding Windows thread priorities; see:
	/// https://msdn.microsoft.com/en-us/library/windows/desktop/ms685100(v=vs.85).aspx
	/// \param[in] integer value representing the NN priority of the thread.
	/// \return integer value representing the Windows priority (as defined in
	///			WinBase.h) of the thread.
	int getNNToWinPriority(const int nnPriority) const;

	///@}

protected:

	/// Windows handle for the thread instance
	HANDLE mThreadHnd;

	/// Integer ID assigned to the thread (some Windows functions rely on this
	/// instead of the handle for referencing a thread)
	DWORD mThreadId;

	/// Thread name
	const char* mName;

	/// Thread priority; integer value between #HighestThreadPriority and
	/// #LowestThreadPriority.
	int mPriority;
};

/// \brief Class that represents a Windows mutex instance.
/// This implementation of NvMutex uses Windows' critical sections (used for
/// mutual exclusion among threads in a Windows process), as opposed to
/// Windows mutexes (used for inter-process mutual exclusion, and come at a
/// much higher performance premium due to the additional system calls).
class NvMutexWin : public NvMutex {

public:

	/// \brief Default constructor
	/// Do NOT use. Overwritten for safety; it should throw an exception.
	NvMutexWin(void);

	/// \brief Copy constructor
	/// Do NOT use. Overwritten for safety; it should throw an exception.
	/// \param[in] obj reference to instance in attempted copy.
	NvMutexWin(const NvMutex& obj);

	/// \brief Constructor
	/// Only constructor that creates an instance of a mutex.
	/// \param[in] recursive flag that determines if the mutex is recursive
	///						 (it allows the same thread to acquire multiple
	///						 levels of ownership over the same mutex)
	/// \param[in] lockLevel maximum number of ownership levels allowed for the
	///						 mutex and the same thread
	NvMutexWin(const bool recursive, const int lockLevel);

	/// \brief Destructor
	virtual ~NvMutexWin(void);

	/// \name Mutex API pure virtual methods
	/// To be overridden with platform specific calls by the implementing
	/// derived classes.
	///@{

	/// \brief Lock the mutex.
	/// The currently executing thread acquires ownership of the mutex. This is
	/// a BLOCKING call (thread execution is stopped until the thread acquires
	/// the mutex, and the method will not return until this is the case).
	void lockMutex(void);

	/// \brief Lock the mutex.
	/// The currently executing thread acquires ownership of the mutex. This is
	/// a NON-BLOCKING call (thread execution is NOT stopped until the thread
	/// acquires the mutex, and the method will return after the attempt).
	/// \return whether the thread acquired the mutex or not.
	bool tryLockMutex(void);

	/// \brief Unlock the mutex
	/// If the mutex is recursive, and the currently executing thread had
	/// acquired ownership of the mutex, then the lock level is decreased by 1.
	/// If the lock level is 0 or if the mutex is non-recursive, and the
	/// currently executing thread had acquired ownership of the mutex, then
	/// the mutex is unlocked and available to other threads.
	void unlockMutex(void);

	/// \brief Check if the mutex is owned by the current thread.
	/// This function is provided mostly for use in error-checking code. This
	/// method does NOT work on Windows (there is no Windows API for this
	/// use-case), and it simply throws an exception.
	/// \return whether the mutex is owned by the current thread.
	bool isMutexLockedByCurrentThread(void);

	///@}

	/// \name Mutex accessor methods
	/// Used mostly by the #NvThreadManager instance to catalog and index
	/// mutexes
	///@{

	/// \brief Get a pointer to the critical section.
	/// Used by the condition variable to access and lock/unlock its
	/// associated mutex when waiting.
	/// \return pointer to the critical section.
	const CRITICAL_SECTION* getCriticalSection(void) const;

	///@}

protected:

	/// Instance of Windows' implementation of an intra-process mutex.
	CRITICAL_SECTION mCriticalSection;

	/// Flag that determines if the mutex is recursive (it allows the same
	/// thread to acquire multiple levels of ownership over the same mutex)
	bool mRecursive;

	/// Maximum number of ownership levels allowed for the mutex and the same
	/// thread
	int mLockLevel;
};

/// \brief Class that represents a Windows' condition variable instance.
class NvConditionVariableWin : public NvConditionVariable {

public:

	/// \brief Default constructor
	/// Only constructor that creates an instance of a condition variable.
	NvConditionVariableWin(void);

	/// \brief Copy constructor
	/// Do NOT use. Overwritten for safety; it should throw an exception.
	/// \param[in] obj reference to instance in attempted copy
	NvConditionVariableWin(const NvConditionVariableWin& obj);

	/// \brief Destructor
	virtual ~NvConditionVariableWin(void);

	/// \name Condition variable API pure virtual methods
	/// To be overridden with platform specific calls by the implementing
	/// derived classes.
	///@{

	/// \brief Notify one thread waiting on the condition variable.
	/// Usually used to dispatch work to other threads after placing it in a
	/// queue (monitor paradigm).
	void signalConditionVariable(void);

	/// \brief Notify all threads waiting on the condition variable.
	/// Usually used to dispatch work to other threads after placing it in a
	/// queue (monitor paradigm).
	void broadcastConditionVariable(void);

	/// \brief Make the thread wait until the mutex is available.
	/// It will also unlock the mutex while the thread sleeps, and it will
	/// reacquire it once it wakes up to check on the condition again.
	/// \param[in] mutex the mutex associated with the condition variable.
	void waitConditionVariable(NvMutex* mutex);

	/// \brief Make the thread wait until the mutex is available for a fixed
	/// amount of time.
	/// It will also unlock the mutex while the thread sleeps, and it will
	/// reacquire it once it wakes up to check on the condition again.
	/// \param[in] mutex the mutex associated with the condition variable.
	/// \param[in] timeout requested waiting time in nanoseconds.
	/// \return the #NvConditionVariableStatus enumeration signaling whether
	///			the timed wait on a condition variable exited due to a timeout
	///			or a successful mutex lock.
	NvConditionVariableStatus timedWaitConditionVariable(
		NvMutex* mutex, long long int timeout);

	///@}

private:

	/// \name Private helper methods
	///@{

	/// \brief Helper method to handle conitional variable waiting.
	/// It will also unlock the mutex while the thread sleeps, and it will
	/// reacquire it once it wakes up to check on the condition again.
	/// \param[in] mutex the mutex associated with the condition variable.
	/// \param[in] timeout requested waiting time in nanoseconds.
	/// \return whether the thread was awakened before the timeout expires.
	bool baseWaitConditionVariable(NvMutex* mutex, long long int timeout);

	///@}

protected:

	/// Instance of Windows' implementation of a condition variable.
	CONDITION_VARIABLE mConditionVariable;
};

/// \brief Exception class
/// Abstract exception class available in some architectures for signaling
/// when the threading API implementation entered an illegal state.
class NvThreadExceptionWin : public NvThreadException {

public:

	/// \brief Returns exception message.
	/// Return a string with details on the illegal state the application
	/// entered.
	/// \return character string with an implementation-dependent exception
	/// message.
	virtual const char* what(void) const throw();
};

/// \brief Windows implementation of the #NvThreadManager class.
/// It is used to create, manage and destroy thread, mutexes and condition
/// variables; these are implemented using Windows native APIs. A single
/// instance of this class should exist for the application.
class NvThreadManagerWin : public NvThreadManager {
public:

	/// \brief Default constructor.
	/// Use it to create a singleton instance of the derived, implementing
	/// class.
	NvThreadManagerWin(void);

	/// \brief Copy constructor
	/// Do NOT use. Overwritten for safety; it should throw an exception.
	/// \param[in] obj reference to instance in attempted copy
	NvThreadManagerWin(const NvThreadManagerWin& obj);

	/// \brief Destructor
	virtual ~NvThreadManagerWin(void);

	/// \name #NvThread related methods.
	/// Used to handle creation, destruction and other non-instance-related
	/// interactions with #NvThread instances.
	///@{

	/// \brief Create an instance of a thread.
	/// Used to facilitate constructor handling as well as tracking all
	/// #NvThread objects by the application.
	/// \param[in] function the function pointer the thread should execute.
	/// \param[in] argument data structure with arguments to the function.
	/// \param[in] stack pointer to allocated stack memory for the thread.
	/// \param[in] stackSize size of allocated stack memory for the thread.
	///						 It must be aligned with the NVTHREAD_STACK_ALIGN
	///						 attribute declaration.
	/// \param[in] priority priority assigned to the thread; integer value
	///						between #HighestThreadPriority and
	///						#LowestThreadPriority.
	/// \return new thread instance.
	NvThread* createThread(NvThreadFunction function, void* argument,
		void* stack, size_t stackSize, int priority);

	/// \brief Destroys an instance of a thread.
	/// This ONLY works if the thread instance has created by calling the
	/// #createThread method, as opposed to calling the #NvThread constructor
	/// directly.
	/// \param[in] thread instance to be destroyed.
	void destroyThread(NvThread* thread);

	/// \brief Yields to a similarly prioritized thread.
	/// Cedes the execution opportunity of the currently executing thread to
	/// another of similar priority.
	void yieldThread(void);

	/// \brief Sleep for a fixed amount of time.
	/// Pauses execution of the currently executing thread for a fixed amount of
	/// time.
	/// \param[in] time sleep time in nanoseconds.
	void sleepThread(long long int time);

	/// \brief Gets the currently executing thread.
	/// This ONLY works if the thread instance has created by calling the
	/// #createThread method, as opposed to calling the #NvThread constructor
	/// directly.
	/// \return pointer to the current thread.
	NvThread* getCurrentThread(void) const;

	/// \brief Gets the current processor number.
	/// Queries and returns the processor number for the currently executing
	/// thread.
	/// \return the processor number.
	int getCurrentProcessorNumber(void) const;

	///@}

	/// \name #NvMutex related methods.
	/// Used to handle initialization and finalization of #NvMutex instances.
	///@{

	/// \brief Create a new mutex instance.
	/// Mutexes can be recursive (i.e. they allows the same thread to acquire
	/// multiple levels of ownership over the same mutex) or not.
	/// \param[in] recursive flag that determines if the mutex is recursive
	/// \param[in] lockLevel maximum number of ownership levels allowed for the
	///						 mutex and the same thread.
	/// \return pointer to the new mutex instance.
	NvMutex* initializeMutex(const bool recursive, const int lockLevel) const;

	/// \brief Destroy a mutex instance.
	/// \param[in] mutex instance to be destroyed.
	void finalizeMutex(NvMutex* mutex) const;

	///@}

	/// \name #NvConditionVariable related methods.
	/// Used to handle initialization and finalization of #NvConditionVariable
	/// instances.
	///@{

	/// \brief Create a new condition variable instance.
	/// \return pointer to the new condition variable instance.
	NvConditionVariable* initializeConditionVariable(void) const;

	/// \brief Destroy a condition variable instance.
	/// \param[in] condition variable instance to be destroyed.
	void finalizeConditionVariable(NvConditionVariable* conditionVariable)
		const;

	///@}

protected:

	/// Container used to store relationships between #NvThreadWin objects and
	/// corresponding Windows thread IDs (for quick retrieval). Used to enable
	// mapping from IDs to #NvThread pointers when calling #getCurrentThread.
	std::unordered_map<DWORD, NvThreadWin*> mThreadMap;
};

#endif
