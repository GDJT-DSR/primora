cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(primora DESCRIPTION "a verifier and generator of prime" LANGUAGES C)

file(GLOB SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/*.c")

# add_compile_options(-fsanitize=address -g)

add_executable(primora "${SOURCES}")
if(MSVC)
    # 屏蔽 sscanf/fgets 等 CRT 函数的 C4996 警告
    target_compile_definitions(primora PRIVATE _CRT_SECURE_NO_WARNINGS)

    target_compile_options(primora PRIVATE
        /utf-8                                   # 源文件是 UTF-8，里面有中文字符串
        $<$<CONFIG:Release>:/O2 /GL>
        $<$<CONFIG:Debug>:/Od /Zi>
    )

    target_link_options(primora PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    target_compile_options(primora PRIVATE
        $<$<CONFIG:Release>:-O3 -march=native -funroll-loops -flto>
        $<$<CONFIG:Debug>:-g -fno-omit-frame-pointer>
    )

    target_link_options(primora PRIVATE
        $<$<CONFIG:Release>:-flto>
    )
endif()

enable_testing()

add_test(NAME VerifySmallPrime COMMAND primora verify 0x2f)
set_tests_properties(VerifySmallPrime PROPERTIES
    LABELS "verify"
    TIMEOUT 10
)

add_test(NAME VerifyTwo COMMAND primora verify 0x2)
set_tests_properties(VerifyTwo PROPERTIES
    LABELS "verify"
    TIMEOUT 10
)

add_test(NAME VerifyComposite COMMAND primora verify 0x2d)
set_tests_properties(VerifyComposite PROPERTIES
    LABELS "verify"
    TIMEOUT 10
    PASS_REGULAR_EXPRESSION "合数"
)

add_test(NAME VerifyCompositeExitCode COMMAND primora verify 0x2d)
set_tests_properties(VerifyCompositeExitCode PROPERTIES
    LABELS "verify"
    TIMEOUT 10
    WILL_FAIL TRUE
)
add_test(NAME VerifyLargePrime COMMAND primora verify 0x4dd9586d28f901760625962b1145d0273b24a1c231027fe250711f351baaa0329d4826416cea051836d10b7c6ef09a22e9522053b80503b41bfbc9377de9d77fe3af0e2f5cbefe7fcce1675784eb74ca80ecf255a1da0d51319589005347383d315d029d75cd565991049f2bc9496dd100aa5a69592e5321ce6f74af54191c8de1b0646a92fec1ee4ce1ef600dc6bddd2c27bb55d99facac74af4384293474fa87bae72fd8b5710f6886067cf233e3450f724cf4b2ab38d802c6a6cc36075991d17e8cc838ddc01416e14ab6d119cc70914eaee479320f15e01fde0be485ed87448d217720f977d822b5bff5155c838f3cad28578bde1c28a07857bcd2405ebabf189d3634d64782e8c1b545073f11e8e55b5693e4d0e1030cce191a823837765f789a40d05c08e039a9a3d568d053096e00eb4e511c142b9f9a2afd5dd065b75fbc95fddd3c1cbea274b207ab6963d121213abcec4793ea22bfc801a79f9965f33816f127622cd7f3934ec9be05e5e23f58c3f82cba69f21faa27f6f67de25c9c0c45c4f97d6b00047eb429c07291dda035dae3db4a01b4223f9677256e0899af4542bcf663de4096009eca4a573170297316c5577d55bd08b2af4a25410ada6116f0486f14c975d10e9497033936a599b98aeec8df46f4fae4673801999a938e2f464a9a2afac6d3dc399f8ece59e1551b4414165c77c5fbdbaecb998a9e89)
set_tests_properties(VerifyLargePrime PROPERTIES
    LABELS "verify;slow"
    TIMEOUT 300
)
add_test(NAME GenerateSmall COMMAND primora gen 64 -s 20 -o)
set_tests_properties(GenerateSmall PROPERTIES
    LABELS "gen"
    TIMEOUT 60
    PASS_REGULAR_EXPRESSION "Result:"
)

add_test(NAME GenerateLarge COMMAND primora gen 1024 -s 6975 -o)
set_tests_properties(GenerateLarge PROPERTIES
    LABELS "gen;slow"
    TIMEOUT 120
    PASS_REGULAR_EXPRESSION "Result:"
)
add_test(NAME UsageNoArgs COMMAND primora)
set_tests_properties(UsageNoArgs PROPERTIES
    LABELS "cli"
    TIMEOUT 10
    WILL_FAIL TRUE
)
