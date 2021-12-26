#include "dllMain.h"

FREContext ctx;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#include "win32.h"
#endif

void contextInitializer(void *extData, const uint8_t * ctxType, FREContext aneCtx, uint32_t * numFunctionsToSet, const FRENamedFunction ** functionsToSet)
{
	ctx = aneCtx;

	// Create mapping between function names and pointers in an array of FRENamedFunction.
	// These are the functions that you will call from ActionScript -
	// effectively the interface of your native library.
	// Each member of the array contains the following information:
	// { function name as it will be called from ActionScript,
	//   any data that should be passed to the function,
	//   a pointer to the implementation of the function in the native library }
	static FRENamedFunction extensionFunctions[] =
	{
		{}
		//{ (const uint8_t*)"coreInitialization", NULL, &coreInitialization },
	};

	// Tell AIR how many functions there are in the array:
	*numFunctionsToSet = sizeof(extensionFunctions) / sizeof(FRENamedFunction);

	// Set the output parameter to point to the array we filled in:
	*functionsToSet = extensionFunctions;
}

void contextFinalizer(FREContext ctx)
{
	return;
}

//extern "C" {

	void FractalServerExtensionInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer){
		*ctxInitializer = &contextInitializer; // The name of function that will intialize the extension context
		*ctxFinalizer = &contextFinalizer; // The name of function that will finalize the extension context
	}

	void FractalServerExtensionFinalizer(void* extData){
		return;
	}

//}

template<typename RESP>
RESP init_resp(RESP resp){
	resp.append_header(restinio::http_field::server, "FractalServer version 0.1");
	resp.append_header_date_field();
	return resp;
}

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

auto server_handler(){
	auto router = std::make_unique<router_t>();
	router->http_get("/", []( auto req, auto ){
				init_resp( req->create_response() )
					.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
					.set_body("")
					.done();
				return restinio::request_accepted();
	});
}

void start(std::string privateKey, std::string serverKey, std::string pemKey){
	using namespace std::chrono;
	try
	{
		using traits_t = restinio::single_thread_tls_traits_t<restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,	router_t>;

		// Since RESTinio supports both stand-alone ASIO and boost::ASIO
		// we specify an alias for a concrete asio namesace.
		// That's makes it possible to compile the code in both cases.
		// Typicaly only one of ASIO variants would be used,
		// and so only asio::* or only boost::asio::* would be applied.
		namespace asio_ns = restinio::asio_ns;

		asio_ns::ssl::context tls_context{ asio_ns::ssl::context::sslv23 };
		tls_context.set_options(asio_ns::ssl::context::default_workarounds | asio_ns::ssl::context::no_sslv2 | asio_ns::ssl::context::single_dh_use);

		tls_context.use_certificate_chain_file(serverKey);
		tls_context.use_private_key_file(privateKey, asio_ns::ssl::context::pem); // certs_dir + "/key.pem",
		tls_context.use_tmp_dh_file(pemKey);

		restinio::run(
			restinio::on_this_thread<traits_t>()
			.address("localhost")
			.port(443)
			.request_handler(server_handler)
			.read_next_http_message_timelimit(10s)
			.write_http_response_timelimit(10s)
			.handle_request_timeout(20s)
			.tls_context(std::move(tls_context))
		);
	}
	catch( const std::exception & ex )
	{
		// std::cerr << "Error: " << ex.what() << std::endl;
	}

}