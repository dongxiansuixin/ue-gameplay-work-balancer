#include "BlueprintGraph/GWBWildcardValueCache.h"


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
TMap<FString, int32> UGWBWildcardValueCache::IntCache;
TMap<FString, float> UGWBWildcardValueCache::FloatCache;
TMap<FString, FString> UGWBWildcardValueCache::StringCache;
TMap<FString, bool> UGWBWildcardValueCache::BoolCache;
TMap<FString, TWeakObjectPtr<UObject>> UGWBWildcardValueCache::ObjectCache;
TMap<FString, FVector> UGWBWildcardValueCache::VectorCache;
TMap<FString, FRotator> UGWBWildcardValueCache::RotatorCache;
TMap<FString, FTransform> UGWBWildcardValueCache::TransformCache;
FCriticalSection UGWBWildcardValueCache::CacheLock;

// Set functions
void UGWBWildcardValueCache::SetWildcardInt(const FString& Key, int32 Value)
{
	FScopeLock Lock(&CacheLock);
	IntCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardFloat(const FString& Key, float Value)
{
	FScopeLock Lock(&CacheLock);
	FloatCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardString(const FString& Key, const FString& Value)
{
	FScopeLock Lock(&CacheLock);
	StringCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardBool(const FString& Key, bool Value)
{
	FScopeLock Lock(&CacheLock);
	BoolCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardObject(const FString& Key, UObject* Value)
{
	FScopeLock Lock(&CacheLock);
	ObjectCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardVector(const FString& Key, const FVector& Value)
{
	FScopeLock Lock(&CacheLock);
	VectorCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardRotator(const FString& Key, const FRotator& Value)
{
	FScopeLock Lock(&CacheLock);
	RotatorCache.Add(Key, Value);
}

void UGWBWildcardValueCache::SetWildcardTransform(const FString& Key, const FTransform& Value)
{
	FScopeLock Lock(&CacheLock);
	TransformCache.Add(Key, Value);
}

// Get functions
int32 UGWBWildcardValueCache::GetWildcardInt(const FString& Key, int32 DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (int32* FoundValue = IntCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

float UGWBWildcardValueCache::GetWildcardFloat(const FString& Key, float DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (float* FoundValue = FloatCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

FString UGWBWildcardValueCache::GetWildcardString(const FString& Key, const FString& DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (FString* FoundValue = StringCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

bool UGWBWildcardValueCache::GetWildcardBool(const FString& Key, bool DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (bool* FoundValue = BoolCache.Find(Key))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

UObject* UGWBWildcardValueCache::GetWildcardObject(const FString& Key, UObject* DefaultValue)
{
	FScopeLock Lock(&CacheLock);
	if (TWeakObjectPtr<UObject>* FoundValue = ObjectCache.Find(Key))
	{
		return FoundValue->IsValid() ? FoundValue->Get() : DefaultValue;
	}
	return DefaultValue;
}

FVector UGWBWildcardValueCache::GetWildcardVector(const FString& Key)
{
	FScopeLock Lock(&CacheLock);
	if (FVector* FoundValue = VectorCache.Find(Key))
	{
		return *FoundValue;
	}
	return FVector::ZeroVector;
}

FRotator UGWBWildcardValueCache::GetWildcardRotator(const FString& Key)
{
	FScopeLock Lock(&CacheLock);
	if (FRotator* FoundValue = RotatorCache.Find(Key))
	{
		return *FoundValue;
	}
	return FRotator::ZeroRotator;
}

FTransform UGWBWildcardValueCache::GetWildcardTransform(const FString& Key)
{
	FScopeLock Lock(&CacheLock);
	if (FTransform* FoundValue = TransformCache.Find(Key))
	{
		return *FoundValue;
	}
	return FTransform::Identity;
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

bool UGWBWildcardValueCache::HasWildcardKey(const FString& Key, const FName& ValueType)
{
	FScopeLock Lock(&CacheLock);
	
	if (ValueType == PC_Int)
	{
		return IntCache.Contains(Key);
	}
	else if (ValueType == PC_Float)
	{
		return FloatCache.Contains(Key);
	}
	else if (ValueType == PC_String)
	{
		return StringCache.Contains(Key);
	}
	else if (ValueType == PC_Boolean)
	{
		return BoolCache.Contains(Key);
	}
	else if (ValueType == PC_Object)
	{
		return ObjectCache.Contains(Key);
	}
	else if (ValueType == TEXT("Vector") || (ValueType == PC_Struct && ValueType.ToString().Contains(TEXT("Vector"))))
	{
		return VectorCache.Contains(Key);
	}
	else if (ValueType == TEXT("Rotator") || (ValueType == PC_Struct && ValueType.ToString().Contains(TEXT("Rotator"))))
	{
		return RotatorCache.Contains(Key);
	}
	else if (ValueType == TEXT("Transform") || (ValueType == PC_Struct && ValueType.ToString().Contains(TEXT("Transform"))))
	{
		return TransformCache.Contains(Key);
	}
	
	return false;
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