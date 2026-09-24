#pragma once

#include <atomic>
#include <thread>

namespace logging {

    inline std::atomic<bool> g_spinning{ false };
    inline std::thread g_spin_thread;
    inline char g_spin_label[ 96 ]{};

    inline HANDLE con_out( ) {
        return GetStdHandle( STD_OUTPUT_HANDLE );
    }

    inline void cursor_visible( bool show ) {
        CONSOLE_CURSOR_INFO ci{};
        ci.dwSize = 1;
        ci.bVisible = show ? TRUE : FALSE;
        SetConsoleCursorInfo( con_out( ), &ci );
    }

    inline void stamp( ) {
        auto now = std::chrono::system_clock::now( );
        std::time_t time = std::chrono::system_clock::to_time_t( now );
        tm local_tm{};
        localtime_s( &local_tm, &time );
        printf( oxorany( "\x1b[38;2;0;90;40m[%02d/%02d/%04d %02d:%02d:%02d]\x1b[0m " ),
            local_tm.tm_mon + 1,
            local_tm.tm_mday,
            local_tm.tm_year + 1900,
            local_tm.tm_hour,
            local_tm.tm_min,
            local_tm.tm_sec );
        printf( oxorany( "\x1b[1m\x1b[38;2;57;255;20m[EL]\x1b[0m " ) );
    }

    inline void spin_stop( ) {
        const bool was = g_spinning.exchange( false );
        if ( g_spin_thread.joinable( ) )
            g_spin_thread.join( );
        if ( was ) {
            printf( "\r\x1b[2K" );
            fflush( stdout );
        }
        cursor_visible( true );
    }

    inline void spin( const char* label ) {
        spin_stop( );
        strncpy_s( g_spin_label, label ? label : "", _TRUNCATE );
        g_spinning = true;
        cursor_visible( false );
        g_spin_thread = std::thread( [ ] {
            const char frames[ ] = { '|', '/', '-', '\\' };
            int i = 0;
            while ( g_spinning.load( std::memory_order_relaxed ) ) {
                stamp( );
                printf( "\x1b[1m\x1b[38;2;57;255;20m[%c]\x1b[0m \x1b[38;2;160;255;160m%-52s\x1b[0m\r",
                    frames[ i & 3 ], g_spin_label );
                fflush( stdout );
                ++i;
                Sleep( 80 );
            }
        } );
    }

    constexpr int k_banner_inner = 44;

    inline void banner_bar( bool top ) {
        printf( "    \x1b[38;2;57;255;20m%c", top ? '\xDA' : '\xC0' );
        for ( int i = 0; i < k_banner_inner; ++i )
            putchar( '\xC4' );
        printf( "%c\x1b[0m\n", top ? '\xBF' : '\xD9' );
    }

    inline void banner_fill( ) {
        printf( "    \x1b[38;2;57;255;20m\xB3" );
        for ( int i = 0; i < k_banner_inner; ++i )
            putchar( ' ' );
        printf( "\xB3\x1b[0m\n" );
    }

    inline void banner_line( const char* text, bool title ) {
        const int len = static_cast< int >( strlen( text ) );
        const int left = 3;
        int right = k_banner_inner - left - len;
        if ( right < 0 )
            right = 0;

        printf( "    \x1b[38;2;57;255;20m\xB3" );
        for ( int i = 0; i < left; ++i )
            putchar( ' ' );
        if ( title )
            printf( "\x1b[1m\x1b[38;2;190;255;160m%s\x1b[0m\x1b[38;2;57;255;20m", text );
        else
            printf( "\x1b[38;2;0;200;90m%s\x1b[38;2;57;255;20m", text );
        for ( int i = 0; i < right; ++i )
            putchar( ' ' );
        printf( "\xB3\x1b[0m\n" );
    }

    inline void banner( ) {
        printf( "\n" );
        banner_bar( true );
        banner_fill( );
        banner_line( oxorany( "ELEVATION" ), true );
        banner_line( oxorany( "elevation mapper" ), false );
        banner_line( oxorany( "ntoskrnl-backed execution" ), false );
        banner_fill( );
        banner_bar( false );
        printf( "\n" );
    }

    template<typename... Args>
    inline void print( const char* format, Args... args ) {
        spin_stop( );
        stamp( );
        printf( oxorany( "\x1b[38;2;160;255;160m" ) );
        printf( format, args... );
        printf( oxorany( "\x1b[0m\n" ) );
    }

    template<typename... Args>
    inline void money( const char* format, Args... args ) {
        spin_stop( );
        stamp( );
        printf( oxorany( "\x1b[1m\x1b[38;2;57;255;20m" ) );
        printf( format, args... );
        printf( oxorany( "\x1b[0m\n" ) );
    }
}
