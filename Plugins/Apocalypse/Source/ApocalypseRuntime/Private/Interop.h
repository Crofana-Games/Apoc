// Copyright Crofana Games. All Rights Reserved.

#pragma once

// Dummy object that provides operations on managed object.
// This class is not instantiatable and cannot have any data member, it only use the GCHandle internal value as address.
class FManagedObject
{
	FManagedObject() = delete;
	FManagedObject(const FManagedObject&) = delete;
	FManagedObject(FManagedObject&&) = delete;
	~FManagedObject() = delete;
	FManagedObject& operator=(const FManagedObject&) = delete;
	FManagedObject& operator=(FManagedObject&&) = delete;
};

static_assert(sizeof(FManagedObject) == 1, "FManagedObject can't have any data member so does vtable!!!");

class FManagedType : FManagedObject
{
	FManagedType() = delete;
};

template <typename T>
struct TBridge
{
	TMap<FManagedObject*, T> Unmanaged2Managed;
	TMap<T, FManagedObject*> Managed2Unmanaged;
};

class FDotNetRuntime
{
	TBridge<FWeakObjectPtr> ObjectLookup;
};