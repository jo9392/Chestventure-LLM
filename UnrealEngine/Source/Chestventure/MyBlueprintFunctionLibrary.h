#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "MyBlueprintFunctionLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnOpenAIResponse, const FString&, ResponseText);

/**
 * 
 */
UCLASS()
class CHESTVENTURE_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "Utility")
    static void SendMessageToOpenAI(const FString& UserInput, FOnOpenAIResponse OnResponseCallback);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    static void PrintToScreen(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    static FString GetStringMessage();
};
