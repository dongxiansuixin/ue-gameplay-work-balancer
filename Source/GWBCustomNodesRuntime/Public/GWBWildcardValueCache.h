#pragma once

#include "CoreMinimal.h"
#include "DataTypes/GWBWorkUnitHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "GWBWildcardValueCache.generated.h"


UCLASS()
class GWBCUSTOMNODESRUNTIME_API UGWBWildcardValueCache : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	static const FName PC_Boolean;
	static const FName PC_Byte;
	static const FName PC_Class;    // SubCategoryObject is the MetaClass of the Class passed thru this pin, or SubCategory can be 'self'. The DefaultValue string should always be empty, use DefaultObject.
	static const FName PC_SoftClass;
	static const FName PC_Int;
	static const FName PC_Int64;
	static const FName PC_Float;
	static const FName PC_Double;
	static const FName PC_Real;
	static const FName PC_Name;
	static const FName PC_Delegate;    // SubCategoryObject is the UFunction of the delegate signature
	static const FName PC_MCDelegate;  // SubCategoryObject is the UFunction of the delegate signature
	static const FName PC_Object;    // SubCategoryObject is the Class of the object passed thru this pin, or SubCategory can be 'self'. The DefaultValue string should always be empty, use DefaultObject.
	static const FName PC_Interface;	// SubCategoryObject is the Class of the object passed thru this pin.
	static const FName PC_SoftObject;		// SubCategoryObject is the Class of the AssetPtr passed thru this pin.
	static const FName PC_String;
	static const FName PC_Text;
	static const FName PC_Struct;    // SubCategoryObject is the ScriptStruct of the struct passed thru this pin, 'self' is not a valid SubCategory. DefaultObject should always be empty, the DefaultValue string may be used for supported structs.
	static const FName PC_Enum;    // SubCategoryObject is the UEnum object passed thru this pin

public:
	// Set functions for different value types
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardInt(const int32 Key, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardFloat(const int32 Key, float Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardString(const int32 Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardBool(const int32 Key, bool Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardObject(const int32 Key, UObject* Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardVector(const int32 Key, const FVector& Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardRotator(const int32 Key, const FRotator& Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardTransform(const int32 Key, const FTransform& Value);

	// Get functions for different value types
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static int32 GetWildcardInt(const int32 Key, int32 DefaultValue = 0);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static float GetWildcardFloat(const int32 Key, float DefaultValue = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetWildcardString(const int32 Key, const FString& DefaultValue = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static bool GetWildcardBool(const int32 Key, bool DefaultValue = false);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static UObject* GetWildcardObject(const int32 Key, UObject* DefaultValue = nullptr);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FVector GetWildcardVector(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FRotator GetWildcardRotator(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FTransform GetWildcardTransform(const int32 Key);

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void ClearWildcardCache(const FName& ValueType);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void ClearAllWildcardCaches();

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static bool HasWildcardKey(const int32 Key, const FName& ValueType);

	// Function name retrieval methods
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetAssignFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetFindFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

	UFUNCTION(BlueprintPure, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static int32 GetWorkUnitHandleId(UPARAM(ref) const FGWBWorkUnitHandle& Handle) { return Handle.GetId(); }

private:
	// Static maps for each value type
	static TMap<int32, int32> IntCache;
	static TMap<int32, float> FloatCache;
	static TMap<int32, FString> StringCache;
	static TMap<int32, bool> BoolCache;
	static TMap<int32, TWeakObjectPtr<UObject>> ObjectCache;
	static TMap<int32, FVector> VectorCache;
	static TMap<int32, FRotator> RotatorCache;
	static TMap<int32, FTransform> TransformCache;

	// Critical section for thread safety
	static FCriticalSection CacheLock;
};