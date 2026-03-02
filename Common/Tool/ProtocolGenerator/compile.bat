@echo off
REM 현재 배치 파일이 있는 디렉토리를 기준으로 경로 설정
SET BASE_DIR=%~dp0
SET PROTO_DIR=%BASE_DIR%protos
SET OUTPUT_DIR=%BASE_DIR%generated
REM protoc.exe 경로 설정 (PATH에 없거나 특정 버전 사용 시)
REM 예: SET PROTOC_PATH=C:\vcpkg\installed\x64-windows-static\tools\protobuf\protoc.exe
REM 아래 줄에서 %PROTOC_PATH% 를 사용하거나, PATH에 있다면 그냥 protoc 호출
SET PROTOC_CMD=protoc

echo Base Directory: %BASE_DIR%
echo Proto Source Directory: %PROTO_DIR%
echo C++ Output Directory: %OUTPUT_DIR%

REM 출력 디렉토리 생성 (없으면)
if not exist "%OUTPUT_DIR%" (
    echo Creating output directory: %OUTPUT_DIR%
    mkdir "%OUTPUT_DIR%"
)

echo Running protoc...

REM protoc 실행
REM -I 옵션: import 경로 지정 (protos 디렉토리)
REM --cpp_out 옵션: C++ 코드 생성 경로 지정 (generated 디렉토리)
REM 마지막 인자들: 컴파일할 .proto 파일 목록 (공백으로 구분)
%PROTOC_CMD% -I="%PROTO_DIR%" --cpp_out="%OUTPUT_DIR%" "%PROTO_DIR%\enum.proto" "%PROTO_DIR%\protocol.proto" "%PROTO_DIR%\sprotocol.proto"

REM gRPC 코드가 필요하다면 --grpc_out 및 플러그인 설정 추가
REM 예: --grpc_out="%OUTPUT_DIR%" --plugin=protoc-gen-grpc=path/to/grpc_cpp_plugin.exe

if %errorlevel% == 0 (
    echo Protoc completed successfully. Generated files are in %OUTPUT_DIR%
) else (
    echo Protoc failed with error code %errorlevel%
)

REM 창이 바로 닫히는 것을 방지하려면 아래 줄의 주석 해제
REM pause