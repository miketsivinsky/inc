#if !defined(TQUEUE_H)
#define TQUEUE_H

#include <queue>

#if defined(ENA_FW_QT)
	#include <QtCore/QQueue>
	#include <QtCore/QMutex>
	#include <QtCore/QMutexLocker>
	#include <QtCore/QReadWriteLock>
	#include <QtCore/QReadLocker>
	#include <QtCore/QWriteLocker>
#endif

#if defined(_WIN32) && defined(ENA_WIN_API)
	#include <windows.h>
#endif

//*****************************************************************************

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <class T> class TQueueSl
{
	private:
		std::queue<T> mQueue;

	protected:
		typedef typename std::queue<T>::size_type SizeType_;

		TQueueSl() : mQueue() {}
		~TQueueSl() {}

		bool empty_() const { return mQueue.empty(); }
		SizeType_ size_() const { return mQueue.size(); }
		void put_(const T& obj) { mQueue.push(obj); }
                void pop_() { mQueue.pop(); }

		bool readFront_(T& obj)
		{
			if(empty_())
				return false;
			obj = mQueue.front();
			return true;
		}

		bool get_(T& obj)
		{
			if(empty_())
				return false;
			obj = mQueue.front();
			mQueue.pop();
			return true;
		}
};

#if defined(ENA_FW_QT)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <class T> class TQueueQt
{
	private:
		QQueue<T> mQueue;

	protected:
		typedef int SizeType_;

		TQueueQt() : mQueue() {}
		~TQueueQt() {}

		bool empty_() const { return mQueue.isEmpty(); }
		SizeType_ size_() const { return mQueue.count(); }
		void put_(const T& obj) { mQueue.enqueue(obj); }
                void pop_() { mQueue.pop_front(); }

		bool readFront_(T& obj)
		{
			if(empty_())
				return false;
			obj = mQueue.head();
			return true;
		}

		bool get_(T& obj)
		{
			if(empty_())
				return false;
			obj = mQueue.dequeue(); // 1
			return true;
		}
};
#endif

//*****************************************************************************

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class TNoGuard
{
	protected:
		//-----------------------------------------------------------
		class TLocker
		{
			public:
				TLocker(TNoGuard&) {}
		};

		//-----------------------------------------------------------
		class TReadLocker : public TLocker
		{
			public:
				TReadLocker(TNoGuard& guard) : TLocker(guard) {}
		};

		//-----------------------------------------------------------
		class TWriteLocker : public TLocker
		{
			public:
				TWriteLocker(TNoGuard& guard) : TLocker(guard) {}
		};

		TNoGuard() {}
		~TNoGuard() {}
};

#if defined(_WIN32) && defined(ENA_WIN_API)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class TWinCsGuard
{
	private:
		::CRITICAL_SECTION mCs;

	public:
	//protected:
		//-----------------------------------------------------------
		class TLocker
		{
			private:
				TWinCsGuard& mGuard;

			public:
				TLocker(TWinCsGuard& guard) : mGuard(guard) { mGuard.enterCs(); }
				~TLocker() { mGuard.leaveCs();}
		};

		//-----------------------------------------------------------
		class TReadLocker : public TLocker
		{
			public:
				TReadLocker(TWinCsGuard& guard) : TLocker(guard) {}
		};

		//-----------------------------------------------------------
		class TWriteLocker : public TLocker
		{
			public:
				TWriteLocker(TWinCsGuard& guard) : TLocker(guard) {}
		};

		TWinCsGuard() { ::InitializeCriticalSection(&mCs); }
		~TWinCsGuard() { ::DeleteCriticalSection(&mCs); }
		void enterCs() { ::EnterCriticalSection(&mCs); }
		void leaveCs() { ::LeaveCriticalSection(&mCs); }
};
#endif

#if defined(ENA_FW_QT)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class TQtMutexGuard
{
	private:
		QMutex mMutex;

	//protected:
	public:
		//-----------------------------------------------------------
		class TLocker
		{
			private:
				QMutexLocker mMutexLocker;

			public:
				TLocker(TQtMutexGuard& guard) : mMutexLocker(&guard.mMutex) { }
		};

		//-----------------------------------------------------------
		class TReadLocker : public TLocker
		{
			public:
				TReadLocker(TQtMutexGuard& guard) : TLocker(guard) {}
		};

		//-----------------------------------------------------------
		class TWriteLocker : public TLocker
		{
			public:
				TWriteLocker(TQtMutexGuard& guard) : TLocker(guard) {}
		};

        TQtMutexGuard() : mMutex(QMutex::Recursive) {}
		~TQtMutexGuard() {}
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class TQtReadWriteLockGuard
{
	private:
		QReadWriteLock mLock;

	protected:
		//-----------------------------------------------------------
		class TReadLocker
		{
			private:
				QReadLocker mReadLocker;

			public:
                TReadLocker(TQtReadWriteLockGuard& guard) : mReadLocker(&guard.mLock) {}
		};
		//-----------------------------------------------------------
		class TWriteLocker
		{
			private:
				QWriteLocker mWriteLocker;

			public:
                TWriteLocker(TQtReadWriteLockGuard& guard) : mWriteLocker(&guard.mLock) {}
		};

        TQtReadWriteLockGuard() : mLock(QReadWriteLock::NonRecursive) {}
        ~TQtReadWriteLockGuard() {}
};
#endif

//*****************************************************************************

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template
<
	typename T,
	template <typename> class TQueueType,
	typename TGuardType
>
class TQueue : public TQueueType<T>, public TGuardType
{
	protected:
        TQueue& selfNoConst() const  { return const_cast</*typename*/ TQueue<T,TQueueType,TGuardType>&>(*this); }

	public:
		typedef typename TQueue::SizeType_ SizeType;

		explicit TQueue() : TQueueType<T>(), TGuardType() { }
		virtual ~TQueue() {}
		bool empty() const;
		SizeType size() const;
		void put(const T& obj);
                void pop();
                bool readFront(T& obj);
		bool get(T& obj);
};

//-----------------------------------------------------------------------------
template
<
	typename T,
	template <typename> class TQueueType,
	typename TGuardType
>
inline bool TQueue<T,TQueueType,TGuardType>::empty() const
{
	typename TGuardType::TReadLocker locker(selfNoConst());
    return this->empty_();
}

//-----------------------------------------------------------------------------
template
<
	typename T,
	template <typename> class TQueueType,
	typename TGuardType
>
inline typename TQueue<T,TQueueType,TGuardType>::SizeType TQueue<T,TQueueType,TGuardType>::size() const
{
	typename TGuardType::TReadLocker locker(selfNoConst());
    return this->size_();
}

//-----------------------------------------------------------------------------
template
<
	typename T,
	template <typename> class TQueueType,
	typename TGuardType
>
inline void TQueue<T,TQueueType,TGuardType>::put(const T& obj)
{
	typename TGuardType::TWriteLocker locker(selfNoConst());
    this->put_(obj);
}

//-----------------------------------------------------------------------------
template
<
        typename T,
        template <typename> class TQueueType,
        typename TGuardType
>
inline void TQueue<T,TQueueType,TGuardType>::pop()
{
        typename TGuardType::TWriteLocker locker(selfNoConst());
        this->pop_();
}

//-----------------------------------------------------------------------------
template
<
	typename T,
	template <typename> class TQueueType,
	typename TGuardType
>
inline bool TQueue<T,TQueueType,TGuardType>::readFront(T& obj)
{
	typename TGuardType::TReadLocker locker(selfNoConst()); // ? read or write locker
    return this->readFront_(obj);
}

//-----------------------------------------------------------------------------
template
<
	typename T,
	template <typename> class TQueueType,
	typename TGuardType
>
inline bool TQueue<T,TQueueType,TGuardType>::get(T& obj)
{
	typename TGuardType::TWriteLocker locker(selfNoConst());
    return this->get_(obj);
}

#endif // TQUEUE_H


