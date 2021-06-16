#include "utils/xorstr.hpp"
#include "utils/Logging/Logging.hpp"

#include "kiface/kiface.hpp"
#include "kiface/driver_resource.hpp"

#include "eft/structs/structs.hpp"
#include "eft/eft.hpp"
#include "eft/player/player.hpp"
#include "eft/entity/entity.hpp"

#include "menu/menu.hpp"

int main(  )
{
	if ( !GetConsoleWindow( ) ) {
		AllocConsole( );
		freopen_s( reinterpret_cast< FILE** >( stdin ), xorget( "CONIN$" ), xorget( "r" ), stdin );
		freopen_s( reinterpret_cast< FILE** >( stdout ), xorget( "CONOUT$" ), xorget( "w" ), stdout );
	}

	//auto licenser = Licenser(xorget(""));

	//#ifndef NO_AUTH

	//const char* rsa_public = "";
	//if (!licenser.load_rsa_public_key(rsa_public))
	//printf( xorget( "Failed to load public key" ) );
	//bool logged_in = false;
	//if (!licenser.user_login(username, password))
	//	exit(1);
	//else
	//	logged_in = true;

	//system( xorget( "cls" ) );

	//ShowWindow( GetConsoleWindow( ), SW_HIDE );

	if ( !OMG::KERNAL::status( ) )
		printf( "driver not found\n" );

	while ( !OMG::KERNAL::get_pid( xorget( "EscapeFromTarkov.exe" ) ) ) {
		std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
	}

	auto pid = OMG::KERNAL::get_pid( xorget( "EscapeFromTarkov.exe" ) );

	OMG::KERNAL::attach( _( "EscapeFromTarkov.exe" ), pid );

	Log::Info( xorget( "Initialized" ) );

	menu menu_ = menu( );
}