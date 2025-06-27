#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "GWBWildcardValueCache.generated.h"


UCLASS()
class GWBEDITOR_API UGWBWildcardValueCache : public UBlueprintFunctionLibrary
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
	static void SetWildcardInt(const FString& Key, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardFloat(const FString& Key, float Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardString(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardBool(const FString& Key, bool Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardObject(const FString& Key, UObject* Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardVector(const FString& Key, const FVector& Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardRotator(const FString& Key, const FRotator& Value);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void SetWildcardTransform(const FString& Key, const FTransform& Value);

	// Get functions for different value types
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static int32 GetWildcardInt(const FString& Key, int32 DefaultValue = 0);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static float GetWildcardFloat(const FString& Key, float DefaultValue = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetWildcardString(const FString& Key, const FString& DefaultValue = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static bool GetWildcardBool(const FString& Key, bool DefaultValue = false);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static UObject* GetWildcardObject(const FString& Key, UObject* DefaultValue = nullptr);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FVector GetWildcardVector(const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FRotator GetWildcardRotator(const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FTransform GetWildcardTransform(const FString& Key);

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void ClearWildcardCache(const FName& ValueType);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void ClearAllWildcardCaches();

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static bool HasWildcardKey(const FString& Key, const FName& ValueType);

	// Function name retrieval methods
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetAssignFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetFindFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

private:
	// Static maps for each value type
	static TMap<FString, int32> IntCache;
	static TMap<FString, float> FloatCache;
	static TMap<FString, FString> StringCache;
	static TMap<FString, bool> BoolCache;
	static TMap<FString, TWeakObjectPtr<UObject>> ObjectCache;
	static TMap<FString, FVector> VectorCache;
	static TMap<FString, FRotator> RotatorCache;
	static TMap<FString, FTransform> TransformCache;

	// Critical section for thread safety
	static FCriticalSection CacheLock;
};