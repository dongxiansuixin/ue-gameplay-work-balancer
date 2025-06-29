#pragma once

#include "CoreMinimal.h"
#include "DataTypes/GWBWorkUnitHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "GWBWildcardValueCache.generated.h"


/**
 * The wildcard cache is a set of function meant to be used by custom K2 nodes to "capture" variables values 
 * based on their pin type (PC type) and temporarily store them in a global private array managed by the functions
 * in this library.
 *
 * This allows us to effectively capture a variable's value for latent k2 nodes that may have exec pins that are delayed
 * or deferred to the next stack call. A good example is if we have a loop and a latent K2 node that takes the index
 * of the loop and passes it through as an output intended to be used by a latent exec pin. Without the "captured" cache
 * this library provides, the outgoing latent exec pin would always get the last loop index.
 *
 * So this library is intended to provide a set of static library methods for each in type (for example PC_Boolean, PC_Float, PC_String, etc...)
 * that allow you to SET, GET, and REMOVE a value from an underlying global cache. The values are meant to be stored in a lookup
 * map indexed by an int32 that the K2 node needs to generate per execution in some way in order to retrieve the correct
 * captured value.
 *
 * Because these library methods are intended to be used by K2 nodes, they must be UFUNCTIONS and we need a way to get the name
 * of the function for each pin type (for example PC_Boolean, PC_String, etc). This library also therefore provides methods to look up
 * the names fo the GET, SET, and REMOVE functions by pin type.
 *
 * There is also a unique case we want to handle in the case of structs. K2 pins represent structs as the PC_Struct pin type but
 * they also need to know WHICH struct it is, and to do this they provide a few properties on FEdGraphPinType that give you info on the struct type:
 * - FEdGraphPinType::PinCategory maps to one of the PC types (PC_Struct for example)
 * - FEdGraphPinType::PinSubCategory sometimes provides extra info
 * - FEdGraphPinType::PinSubCategoryObject is what the PC_Struct pin category uses to define the struct by containing a soft pointer to an object
 * that you can compare to a struct to see if it matches. For example you can compare `PinTypePinSubCategoryObject.Get() == TBaseStructure<FVector>::Get()` to see
 * if this pin type is for a FVector struct.
 *
 * Because of this unique STRUCT requirement we also need our function name lookup methods to include a UObject* parameter for the sub-category object we can
 * then compare to the important structures like FVector, FRotator, FTransform, etc.
 */
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

	// Remove functions for different value types
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardInt(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardFloat(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardString(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardBool(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardObject(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardVector(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardRotator(const int32 Key);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardTransform(const int32 Key);

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void ClearWildcardCache(const FName& ValueType);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void ClearAllWildcardCaches();

	// Function name retrieval methods
	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetAssignFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetFindFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static FString GetRemoveFunctionName(const FName& PinCategory, UObject* PinSubCategoryObject);

	UFUNCTION(BlueprintPure, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static int32 GetWorkUnitHandleId(UPARAM(ref) const FGWBWorkUnitHandle& Handle) { return Handle.GetId(); }

	UFUNCTION(BlueprintCallable, Category = "GWB|Wildcard Cache", meta = (CallInEditor = "true"))
	static void RemoveWildcardCacheItem(const int32 Key, const FEdGraphPinType& ValueType);

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