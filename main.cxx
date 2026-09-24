#include <impl/includes.h>

int main( int argc, char** argv ) {
	SetConsoleTitleA( oxorany( "elevation-mapper  |  elevation" ) );
	SetUnhandledExceptionFilter( handler::crash_handler );

	auto std_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	DWORD mode = 0;
	if ( GetConsoleMode( std_handle, &mode ) ) {
		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
		SetConsoleMode( std_handle, mode );
	}
	SetConsoleOutputCP( 437 );

	CONSOLE_FONT_INFOEX cfi{ };
	cfi.cbSize = sizeof( cfi );
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 16;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s( cfi.FaceName, oxorany( L"Consolas" ) );
	SetCurrentConsoleFontEx( GetStdHandle( STD_OUTPUT_HANDLE ), FALSE, &cfi );

	logging::banner( );

	if ( !utility::adjust_privilege( 20 ) )
		return std::getchar( );

	if ( argc < 2 ) {
		logging::print( oxorany( "Usage: elevation-mapper <path-to-driver.sys>" ) );
		return std::getchar( );
	}

	logging::spin( oxorany( "Downloading ntoskrnl PDB" ) );
	if ( !g_pdb->load( ) ) {
		logging::print( oxorany( "Failed to load ntoskrnl PDB (required on this build)" ) );
		return std::getchar( );
	}
	logging::money( oxorany( "Loaded %zu symbols from %s" ),
		g_pdb->m_symbols.size( ), g_pdb->m_module_bare.c_str( ) );

	if ( !g_pdb->get_symbol_address( oxorany( "PiDDBLock" ) ) ||
		 !g_pdb->get_symbol_address( oxorany( "PiDDBCacheTable" ) ) ||
		 !g_pdb->get_symbol_address( oxorany( "KeServiceDescriptorTable" ) ) ||
		 !g_pdb->get_symbol_address( oxorany( "ExAcquireResourceExclusiveLite" ) ) ||
		 !g_pdb->get_symbol_address( oxorany( "ExReleaseResourceLite" ) ) ||
		 !g_pdb->get_symbol_address( oxorany( "RtlLookupElementGenericTableAvl" ) ) ||
		 !g_pdb->get_symbol_address( oxorany( "RtlDeleteElementGenericTableAvl" ) ) ) {
		logging::print( oxorany( "Aborted: required ntoskrnl symbols are missing (PDB mismatch)" ) );
		return std::getchar( );
	}

	if ( !g_service->create( ) ) {
		logging::print( oxorany( "Failed to initialize service" ) );

		return std::getchar( );
	}

	if ( !g_driver->initialize( ) ) {
		logging::print( oxorany( "Failed to open driver device" ) );
		cleanup::early_teardown( );
		return std::getchar( );
	}
	cleanup::g_ctx.device_open = true;

	logging::spin( oxorany( "Scanning physical memory" ) );
	if ( !g_paging->setup( ) ) {
		logging::print( oxorany( "Failed to resolve system directory table base" ) );

		cleanup::full_teardown( false );
		return std::getchar( );
	}
	cleanup::g_ctx.paging_ready = true;

	if ( !g_syscall->setup( ) ) {
		logging::print( oxorany( "Failed to initialize kernel execution" ) );

		cleanup::full_teardown( true );
		return std::getchar( );
	}
	cleanup::g_ctx.syscall_ready = true;

	cleanup::kernel_residual_strip( oxorany( "post-load" ) );

	auto dependency = std::make_shared< map::c_dependency >( argv[ 1 ] );
	if ( !dependency->load( ) ) {
		logging::print( oxorany( "Failed to load target image" ) );
		cleanup::full_teardown( true );
		return std::getchar( );
	}

	auto map = std::make_unique< map::c_map >( dependency );
	if ( !map->create( ) ) {
		logging::print( oxorany( "Failed to map image" ) );
		cleanup::full_teardown( true );
		return std::getchar( );
	}

	if ( !map->execute( ) )
		logging::print( oxorany( "Failed to execute image entry point" ) );

	if ( !g_syscall->verify_restored( ) ) {
		logging::print( oxorany( "SSDT leftover hijack detected after execute" ) );
		if ( !g_syscall->force_restore( ) )
			logging::print( oxorany( "SSDT restore failed; reboot before using this system" ) );
	}

	cleanup::full_teardown( true );
	logging::money( oxorany( "Done" ) );
	return std::getchar( );
}
