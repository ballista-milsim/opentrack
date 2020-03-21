# https://developer.tobii.com/download-packages/stream-engine-4-1-0-for-windows-x86
# https://developer.tobii.com/download-packages/stream-engine-4-1-0-for-windows-x64
set(SDK_TOBII "" CACHE PATH "Tobii SDK path")
if(WIN32 AND Qt5SerialPort_FOUND AND SDK_TOBII)
    otr_module(tracker-ballista)

    target_link_libraries(${self} opentrack-dinput ${Qt5SerialPort_LIBRARIES})
    target_include_directories(${self} SYSTEM PUBLIC ${Qt5SerialPort_INCLUDE_DIRS})

    target_include_directories(${self} SYSTEM PRIVATE "${SDK_TOBII}/include")

    set(dll "${SDK_TOBII}/lib/tobii/tobii_stream_engine.dll")
    set(lib "${SDK_TOBII}/lib/tobii/tobii_stream_engine.lib")

    message(${self})
    message(${dll})
    message(${lib})

    install(FILES ${dll} DESTINATION ${opentrack-hier-pfx})
    target_link_libraries(${self} ${lib})
endif()
