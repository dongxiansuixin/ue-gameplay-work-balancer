#include "GWBWildcardValueCache.h"


const FName UGWBWildcardValueCache::PC_Boolean(TEXT("bool"));
const FName UGWBWildcardValueCache::PC_Byte(TEXT("byte"));
const FName UGWBWildcardValueCache::PC_Class(TEXT("class"));
const FName UGWBWildcardValueCache::PC_Int(TEXT("int"));
const FName UGWBWildcardValueCache::PC_Int64(TEXT("int64"));
const FName UGWBWildcardValueCache::PC_Float(TEXT("float"));
const FName UGWBWildcardValueCache::PC_Double(TEXT("double"));
const FName UGWBWildcardValueCache::PC_Real(TEXT("real"));
const FName UGWBWildcardValueCache::PC_Name(TEXT("name"));
const FName UGWBWildcardValueCache::PC_Delegate(TEXT("delegate"));
const FName UGWBWildcardValueCache::PC_MCDelegate(TEXT("mcdelegate"));
const FName UGWBWildcardValueCache::PC_Object(TEXT("object"));
const FName UGWBWildcardValueCache::PC_Interface(TEXT("interface"));
const FName UGWBWildcardValueCache::PC_String(TEXT("string"));
const FName UGWBWildcardValueCache::PC_Text(TEXT("text"));
const FName UGWBWildcardValueCache::PC_Struct(TEXT("struct"));
const FName UGWBWildcardValueCache::PC_Enum(TEXT("enum"));
const FName UGWBWildcardValueCache::PC_SoftObject(TEXT("softobject"));
const FName UGWBWildcardValueCache::PC_SoftClass(TEXT("softclass"));

// Initialize static members
TMap<int32, int32> UGWBWildcardValueCache::IntCache;
TMap<int32, float> UGWBWildcardValueCache::FloatCache;
TMap<int32, FString> UGWBWildcardValueCache::StringCache;
TMap<int32, bool> UGWBWildcardValueCache::BoolCache;
TMap<int32, TWeakObjectPtr<UObject>> UGWBWildcardValueCache::ObjectCache;
TMap<int32, FVector> UGWBWildcardValueCache::VectorCache;
TMap<int32, FRotator> UGWBWildcardValueCache::RotatorCache;
TMap<int32, FTransform> UGWBWildcardValueCache::TransformCache;
FCriticalSection UGWBWildcardValueCache::CacheLock;

// Set functions
void UGWBWildcardValueCache::SetWildcardInt(const int32 Key, int32 Value)
{
	FScopeLock Lock(&CacheLock);
	IntCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardFloat(const int32 Key, float Value)
{
	FScopeLock Lock(&CacheLock);
	FloatCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardString(const int32 Key, const FString& Value)
{
	FScopeLock Lock(&CacheLock);
	StringCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardBool(const int32 Key, bool Value)
{
	FScopeLock Lock(&CacheLock);
	BoolCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardObject(const int32 Key, UObject* Value)
{
	FScopeLock Lock(&CacheLock);
	ObjectCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardVector(const int32 Key, const FVector& Value)
{
	FScopeLock Lock(&CacheLock);
	VectorCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardRotator(const int32 Key, const FRotator& Value)
{
	FScopeLock Lock(&CacheLock);
	RotatorCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardTransform(const int32 Key, const FTransform& Value)
{
	FScopeLock Lock(&CacheLock);
	TransformCache.Add(Key, Value);
}

// Get functions
int32 UGWBWildcardValueCache::GetWildcardInt(const int32 Key, int32 DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (int32* FoundValue = IntCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

float UGWBWildcardValueCache::GetWildcardFloat(const int32 Key, float DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (float* FoundValue = FloatCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

UE_DISABLE_OPTIMIZATION
FString UGWBWildcardValueCache::GetWildcardString(const int32 Key, const FString& DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	auto Cache = StringCache;
	if (FString* FoundValue = Cache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}
UE_ENABLE_OPTIMIZATION

bool UGWBWildcardValueCache::GetWildcardBool(const int32 Key, bool DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (bool* FoundValue = BoolCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

UObject* UGWBWildcardValueCache::GetWildcardObject(const int32 Key, UObject* DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (TWeakObjectPtr<UObject>* FoundValue = ObjectCache.Find(Key))
	{
		return FoundValue->IsValid() ? FoundValue->Get() : DefaultValue;
	}
	return DefaultValue;
}

FVector UGWBWildcardValueCache::GetWildcardVector(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	if (FVector* FoundValue = VectorCache.Find(Key))
	{
		return *FoundValue;
	}
	return FVector::ZeroVector;
}

FRotator UGWBWildcardValueCache::GetWildcardRotator(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	if (FRotator* FoundValue = RotatorCache.Find(Key))
	{
		return *FoundValue;
	}
	return FRotator::ZeroRotator;
}

FTransform UGWBWildcardValueCache::GetWildcardTransform(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	if (FTransform* FoundValue = TransformCache.Find(Key))
	{
		return *FoundValue;
	}
	return FTransform::Identity;
}

// Remove functions
void UGWBWildcardValueCache::RemoveWildcardInt(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	IntCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardFloat(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	FloatCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardString(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	StringCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardBool(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	BoolCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardObject(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	ObjectCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardVector(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	VectorCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardRotator(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	RotatorCache.Remove(Key);
}

void UGWBWildcardValueCache::RemoveWildcardTransform(const int32 Key)
{
	FScopeLock Lock(&CacheLock);
	TransformCache.Remove(Key);
}

// Utility functions
void UGWBWildcardValueCache::ClearWildcardCache(const FName& ValueType)
{
	FScopeLock Lock(&CacheLock);
	
	if (ValueType == PC_Int)
	{
		IntCache.Empty();
	}
	else if (ValueType == PC_Float)
	{
		FloatCache.Empty();
	}
	else if (ValueType == PC_String)
	{
		StringCache.Empty();
	}
	else if (ValueType == PC_Boolean)
	{
		BoolCache.Empty();
	}
	else if (ValueType == PC_Object)
	{
		ObjectCache.Empty();
	}
	// Handle specific struct types by comparing with literal names
	// Vector, Rotator, and Transform are all PC_Struct category, so we need special handling
	else if (ValueType == TEXT("Vector") || (ValueType == PC_Struct && ValueType.ToString().Contains(TEXT("Vector"))))
	{
		VectorCache.Empty();
	}
	else if (ValueType == TEXT("Rotator") || (ValueType == PC_Struct && ValueType.ToString().Contains(TEXT("Rotator"))))
	{
		RotatorCache.Empty();
	}
	else if (ValueType == TEXT("Transform") || (ValueType == PC_Struct && ValueType.ToString().Contains(TEXT("Transform"))))
	{
		TransformCache.Empty();
	}
}

void UGWBWildcardValueCache::ClearAllWildcardCaches()
{
	FScopeLock Lock(&CacheLock);
	
	IntCache.Empty();
	FloatCache.Empty();
	StringCache.Empty();
	BoolCache.Empty();
	ObjectCache.Empty();
	VectorCache.Empty();
	RotatorCache.Empty();
	TransformCache.Empty();
}

// Function name retrieval methods
FString UGWBWildcardValueCache::GetAssignFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject)
{
	if (PinCategory == PC_Int)
	{
		return TEXT("SetWildcardInt");
	}
	else if (PinCategory == PC_Float)
	{
		return TEXT("SetWildcardFloat");
	}
	else if (PinCategory == PC_String)
	{
		return TEXT("SetWildcardString");
	}
	else if (PinCategory == PC_Boolean)
	{
		return TEXT("SetWildcardBool");
	}
	else if (PinCategory == PC_Object)
	{
		return TEXT("SetWildcardObject");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FVector>::Get())
	{
		return TEXT("SetWildcardVector");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FRotator>::Get())
	{
		return TEXT("SetWildcardRotator");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FTransform>::Get())
	{
		return TEXT("SetWildcardTransform");
	}
	
	return TEXT("");
}

FString UGWBWildcardValueCache::GetFindFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject)
{
	if (PinCategory == PC_Int)
	{
		return TEXT("GetWildcardInt");
	}
	else if (PinCategory == PC_Float)
	{
		return TEXT("GetWildcardFloat");
	}
	else if (PinCategory == PC_String)
	{
		return TEXT("GetWildcardString");
	}
	else if (PinCategory == PC_Boolean)
	{
		return TEXT("GetWildcardBool");
	}
	else if (PinCategory == PC_Object)
	{
		return TEXT("GetWildcardObject");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FVector>::Get())
	{
		return TEXT("GetWildcardVector");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FRotator>::Get())
	{
		return TEXT("GetWildcardRotator");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FTransform>::Get())
	{
		return TEXT("GetWildcardTransform");
	}
	
	return TEXT("");
}

FString UGWBWildcardValueCache::GetRemoveFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject)
{
	if (PinCategory == PC_Int)
	{
		return TEXT("RemoveWildcardInt");
	}
	else if (PinCategory == PC_Float)
	{
		return TEXT("RemoveWildcardFloat");
	}
	else if (PinCategory == PC_String)
	{
		return TEXT("RemoveWildcardString");
	}
	else if (PinCategory == PC_Boolean)
	{
		return TEXT("RemoveWildcardBool");
	}
	else if (PinCategory == PC_Object)
	{
		return TEXT("RemoveWildcardObject");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FVector>::Get())
	{
		return TEXT("RemoveWildcardVector");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FRotator>::Get())
	{
		return TEXT("RemoveWildcardRotator");
	}
	else if (PinCategory == PC_Struct && PinSubCategoryObject == TBaseStructure<FTransform>::Get())
	{
		return TEXT("RemoveWildcardTransform");
	}
	
	return TEXT("");
}

void UGWBWildcardValueCache::RemoveWildcardCacheItem(const int32 Key, const FEdGraphPinType& ValueType)
{
	if (ValueType.PinCategory == PC_Int)
	{
		IntCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_Float)
	{
		FloatCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_String)
	{
		StringCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_Boolean)
	{
		BoolCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_Object)
	{
		ObjectCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_Struct && ValueType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
	{
		VectorCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_Struct && ValueType.PinSubCategoryObject == TBaseStructure<FRotator>::Get())
	{
		RotatorCache.Remove(Key);
	}
	else if (ValueType.PinCategory == PC_Struct && ValueType.PinSubCategoryObject == TBaseStructure<FTransform>::Get())
	{
		TransformCache.Remove(Key);
	}
}
