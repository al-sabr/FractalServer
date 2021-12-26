#include "includes.h"

using namespace std;

FREObject privateKey;
FREObject serverKey;
FREObject pemKey;

extern HMODULE dllHandle;
extern FREContext ctx;

template<typename RESP>
RESP init_resp(RESP resp){
	resp.append_header(restinio::http_field::server, "FractalServer version 0.1");
	resp.append_header_date_field();
	return resp;
}

namespace rr = restinio::router;
namespace asio_ns = restinio::asio_ns;

using router_t = rr::express_router_t<>;
using traits_t = restinio::single_thread_tls_traits_t<restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,	router_t>;

using serverInstance = restinio::http_server_t<traits_t>;
std::optional<serverInstance> server{std::nullopt};

auto server_handler(){
	auto router = std::make_unique<router_t>();
	router->http_get("/", []( restinio::request_handle_t req, auto ){
			const auto body = req->body();
			const auto path = req->header().path();
			const auto query = req->header().query();
			const auto fragment = req->header().fragment();

			std::string recap = (std::string)path;
			FREDispatchStatusEventAsync(ctx, (const uint8_t*)"Phase2", (const uint8_t*)"Good");
			init_resp( req->create_response() )
				.append_header( restinio::http_field::content_type, "text/plain; charset=utf-8" )
				.set_body("")
				.done();
			return restinio::request_accepted();
	});
	return router;
}

void start(std::string privateKey, std::string serverKey, std::string pemKey){
	using namespace std::chrono;
	try
	{
		// Since RESTinio supports both stand-alone ASIO and boost::ASIO
		// we specify an alias for a concrete asio namesace.
		// That's makes it possible to compile the code in both cases.
		// Typicaly only one of ASIO variants would be used,
		// and so only asio::* or only boost::asio::* would be applied.
		

		asio_ns::ssl::context tls_context{ asio_ns::ssl::context::sslv23 };
		tls_context.set_options(asio_ns::ssl::context::default_workarounds | asio_ns::ssl::context::no_sslv2 | asio_ns::ssl::context::single_dh_use);

		tls_context.use_certificate_chain_file(serverKey);
		tls_context.use_private_key_file(privateKey, asio_ns::ssl::context::pem); // certs_dir + "/key.pem",
		tls_context.use_tmp_dh_file(pemKey);

		/* restinio::run(
			restinio::on_this_thread<traits_t>()
			.address("localhost")
			.port(443)
			.request_handler(server_handler())
			.read_next_http_message_timelimit(10s)
			.write_http_response_timelimit(10s)
			.handle_request_timeout(20s)
			.tls_context(std::move(tls_context))
		);
 */
		
		server.emplace(restinio::own_io_context(),
			[&tls_context](auto & settings){
					settings.address("localhost");
					settings.port(443);
					settings.request_handler(server_handler());
					settings.read_next_http_message_timelimit(10s);
					settings.write_http_response_timelimit(10s);
					settings.handle_request_timeout(20s);
					settings.tls_context(std::move(tls_context));
			}
		);

		std::thread restinio_control_thread{[serverLocal = &server.value()]{
				// Use restinio::run to launch RESTinio's server.
				// This run() will return only if server stopped from
				// some other thread.
				restinio::run(
					restinio::on_thread_pool(
							1, // Count of worker threads for RESTinio.
							restinio::skip_break_signal_handling(), // Don't react to Ctrl+C.
							serverLocal
					) // Server to be run.
				);
			}
		};
	}
	catch( const std::exception & ex )
	{
		// std::cerr << "Error: " << ex.what() << std::endl;
		FREDispatchStatusEventAsync(ctx, (const uint8_t*)"error", (const uint8_t*)ex.what());
	}

}


FREObject startServer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{

	FREObject result = NULL;
	FRENewObject((const uint8_t*)"Object", 0, NULL, &result, NULL);

	privateKey = argv[0];
	serverKey = argv[1];
	pemKey = argv[2];

	start("", "", "");

	FREDispatchStatusEventAsync(ctx, (const uint8_t*)"Phase2", (const uint8_t*)"Good");

	FREObject loaderArgs[2];
	/* loaderArgs[0] = checkPolicy;
	loaderArgs[1] = applicationDomain;
 */
	FREObject loaderContext = NULL;
	FRENewObject((const uint8_t*)"flash.system.LoaderContext", 2, loaderArgs, &loaderContext, NULL);
	
	return NULL;

}

FREObject stopServer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
	restinio::initiate_shutdown(server.value());
	//restinio_control_thread.join();
	return NULL;
}