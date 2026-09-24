#pragma once

namespace handler {
    long crash_handler( EXCEPTION_POINTERS* exception_pointers ) {
        const auto* context = exception_pointers->ContextRecord;
        char message[ 1024 ];
        sprintf( message,
            oxorany( "Something went wrong.\n\n"
                "elevation mapper hit an unexpected error and needs to close.\n\n"
                "Try:\n"
                "  - Run as Administrator\n"
                "  - Reboot if you already mapped this session\n"
                "  - Check antivirus is not blocking the mapper\n\n"
                "Still stuck? Contact elevation support\n\n"
                "Crash Details:\n"
                "Build: %s %s\n"
                "Error: 0x%08X at %p\n"
                "Registers: RSP=%016llX RDI=%016llX\n"
                "           RSI=%016llX RBX=%016llX\n"
                "           RDX=%016llX RCX=%016llX\n"
                "           RAX=%016llX RBP=%016llX" ),
            __DATE__, __TIME__,
            exception_pointers->ExceptionRecord->ExceptionCode,
            exception_pointers->ExceptionRecord->ExceptionAddress,
            context->Rsp, context->Rdi,
            context->Rsi, context->Rbx,
            context->Rdx, context->Rcx,
            context->Rax, context->Rbp
        );

        if ( g_syscall )
            g_syscall->emergency_restore( );
        cleanup::full_teardown( false );

        printf( oxorany( "\n" ) );
        logging::print( oxorany( "Unhandled exception; shutting down" ) );
        MessageBoxA( 0, message, "ELEVATION - Unexpected Error", MB_ICONERROR | MB_OK );
        return true;
    }
}
