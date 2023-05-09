// Copyright Crofana Games. All Rights Reserved.

#pragma once

namespace Apocalypse
{
	class FManagedObject;
	
	union FManagedValue
	{
		uint8 U1;
		uint16 U2;
		uint32 U4;
		uint64 U8;

		int8 I1;
		int16 I2;
		int32 I4;
		int64 I8;

		float F4;
		double F8;

		uint8 Bool;

		const WIDECHAR* String;

		FManagedObject* Object;
	};

	static_assert(sizeof(FManagedValue) == sizeof(uint64), "FManagedValue mismatch!");

	// This is a dummy object that provides essential operations on managed object.
	// This class is not instantiatable and cannot have any data member (include vtable).
	// It uses the GCHandle internal value as virtual address to identify itself, so NEVER DEREFERENCE THIS!!!
	class FManagedObject
	{
		FManagedObject() = delete;
		FManagedObject(const FManagedObject&) = delete;
		FManagedObject(FManagedObject&&) = delete;
		~FManagedObject() = delete;
		FManagedObject& operator=(const FManagedObject&) = delete;
		FManagedObject& operator=(FManagedObject&&) = delete;
	};

	static_assert(sizeof(FManagedObject) == sizeof(char), "FManagedObject can't have any data member!!!");

	template <typename T>
	class TManagedObjectRegistry
	{
		static_assert(!TIsDerivedFrom<TDecay<T>, FManagedObject>::Value);

	public:
		void Register(FManagedObject* Managed, const T& Unmanaged)
		{
			Managed2Unmanaged.Emplace(Managed, Unmanaged);
			Unmanaged2Managed.Emplace(Unmanaged, Managed);
		}

		void Unregister(const T& Unmanaged)
		{
			FManagedObject* Managed = nullptr;
			if (Unmanaged2Managed.RemoveAndCopyValue(Unmanaged, Managed))
			{
				ensure(Managed2Unmanaged.Remove(Managed));
			}
		}

		void Unregister(FManagedObject* Managed)
		{
			T Unmanaged;
			if (Managed2Unmanaged.RemoveAndCopyValue(Managed, Unmanaged))
			{
				ensure(Unmanaged2Managed.Remove(Unmanaged));
			}
		}

		FManagedObject* ToManaged(const T& Unmanaged) const
		{
			if (auto Managed = Unmanaged2Managed.Find(Unmanaged))
			{
				return *Managed;
			}

			return nullptr;
		}

		bool ToUnmanaged(FManagedObject* Managed, T& OutUnmanaged) const
		{
			if (auto Unmanaged = Managed2Unmanaged.Find(Managed))
			{
				OutUnmanaged = *Unmanaged;
				return true;
			}

			return false;
		}

	private:
		TMap<FManagedObject*, T> Managed2Unmanaged;
		TMap<T, FManagedObject*> Unmanaged2Managed;
	};
}
