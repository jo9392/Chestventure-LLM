// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

// OpenAI API
FString OpenAI_APIKey = TEXT("Enter your tokens");


void UMyBlueprintFunctionLibrary::PrintToScreen(const FString& Message)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Message);
    }
}

FString UMyBlueprintFunctionLibrary::GetStringMessage()
{
    return TEXT("Hello from Unreal Engine!");
}

void UMyBlueprintFunctionLibrary::SendMessageToOpenAI(const FString& UserInput, FOnOpenAIResponse OnResponseCallback)
{
    // HTTP 요청 생성
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    // API Key 설정
    FString APIKey = OpenAI_APIKey;

    // API URL 및 헤더 설정
    Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + APIKey);

    // 요청 본문 작성
    TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
    RequestData->SetStringField(TEXT("model"), TEXT("gpt-3.5-turbo"));

    // 메시지 배열 작성
    TArray<TSharedPtr<FJsonValue>> MessagesArray;

    // Prompt (시스템 메시지) 추가
    FString PromptText = TEXT("당신은 아레프갈드 왕국에 사는 주민입니다. 용사와 대화하세요. 용사를 경배해야 합니다.");
    TSharedPtr<FJsonObject> PromptMessage = MakeShareable(new FJsonObject);
    PromptMessage->SetStringField(TEXT("role"), TEXT("system"));
    PromptMessage->SetStringField(TEXT("content"), PromptText);
    MessagesArray.Add(MakeShareable(new FJsonValueObject(PromptMessage)));

    // 사용자 발화 추가 (인자로 전달된 텍스트 사용)
    TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject);
    UserMessage->SetStringField(TEXT("role"), TEXT("user"));
    UserMessage->SetStringField(TEXT("content"), UserInput); // 인자로 받은 텍스트 사용
    MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMessage)));

    // Messages 배열을 RequestData에 추가
    RequestData->SetArrayField(TEXT("messages"), MessagesArray);

    // JSON 직렬화
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);
    Request->SetContentAsString(RequestBody);

    // Request Body를 로그로 출력
    UE_LOG(LogTemp, Log, TEXT("Request Body: %s"), *RequestBody);

    // 응답 처리
    Request->OnProcessRequestComplete().BindLambda([OnResponseCallback](FHttpRequestPtr RequestPtr, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (bWasSuccessful && Response.IsValid())
            {
                FString ResponseText = Response->GetContentAsString();
                FString ParsedContent;

                // JSON 응답 파싱 및 content 추출
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseText);

                if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
                {
                    const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
                    if (JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
                    {
                        TSharedPtr<FJsonObject> MessageObject = (*ChoicesArray)[0]->AsObject();
                        if (MessageObject.IsValid() && MessageObject->HasField(TEXT("message")))
                        {
                            TSharedPtr<FJsonObject> MessageContent = MessageObject->GetObjectField(TEXT("message"));
                            if (MessageContent.IsValid() && MessageContent->HasField(TEXT("content")))
                            {
                                ParsedContent = MessageContent->GetStringField(TEXT("content"));
                            }
                        }
                    }
                }

                if (!ParsedContent.IsEmpty())
                {
                    UE_LOG(LogTemp, Log, TEXT("Parsed ChatGPT Response: %s"), *ParsedContent);

                    // Delegate 호출
                    OnResponseCallback.ExecuteIfBound(ParsedContent);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to parse content from response."));
                    OnResponseCallback.ExecuteIfBound(TEXT("Failed to parse content from response."));
                }
            }
            else
            {
                FString ErrorMessage = TEXT("HTTP Request failed.");
                UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);

                // Delegate 호출
                OnResponseCallback.ExecuteIfBound(ErrorMessage);
            }
        });

    // 요청 실행
    Request->ProcessRequest();
}
