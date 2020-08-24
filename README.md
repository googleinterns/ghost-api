# GhOST API

GhOST API exposes a set of services to manage an SDN network. This project is meant to demonstrate the capabilities of USPS API.

## USPS API

USPS API exposes a service for data-plane programming.

# How to use

## Server

### Running the Server

To run the server, build the project using bazel and then run the 'run-server' binary that was generated.
Specify the host and port using abseil flags HOST and PORT.
This will look something like:
```
./bazel-bin/example/usps_api/run-server -HOST="localhost" -PORT=1234
```
### Configuration file
The server uses the configuration file to handle requests accordingly. The config file is specified in json formatted and located at [config.json](example/usps_api/config/config.json). The config file watches for file changes while the server is running and will reload new information on a file save.
#### IP address specification
You may also specify the address to bind to in the configuration file. The abseil flags HOST and PORT take priority over the address in the configuration file.
```
{
    "address": {
        "host": "localhost",
        "port": 1234
    }
}
```
#### Enabling requests
The user can enable whether clients can create, delete, or query a request.
```
{
    "requests": {
        "create": true,
        "delete": false,
        "query": true
    }
}
```
#### SSL Specification
```
{
        "ssl": {
            "enable": true,
            "key": "ssl/server.key",
            "cert": "ssl/server.crt",
            "root": "ssl/ca.crt"
        }
}
```
#### Asynchronous option
The user can toggle the server to be asynchronous by setting the async boolean parameter.
```
{"async": true}
```
#### Specific filter actions
- The deny-list will prohibit requests with matching identifiers from being executed. 
- The allow-list will only permit requests with matching identifiers. 
- The delay-list will delay requests with matching identifiers and will overwrite its activation_time.
- Allow and deny-lists are mutually exclusive but the delay-list can exist with either. Items in the delay list are automatically included in the allow-list.
Note: Place filter actions (deny, allow, or delay) underneath sfcfilter. Place either tunnel or routing identifiers underneath these filter actions. 
```
{
    "sfcfilter": {  
        "deny": {
            "ghost_tunnel_identifier": {
                "ghostlabel": [
                    {
                    "terminal_label": 5678,
                    "service_label": 3456
                    },
                    {
                    "terminal_label": 90210,
                    "service_label": 15213
                    }
                ]
            },
            "ghost_routing_identifier": {
                "destination_label_prefix": [
                    {
                    "value": 2342,
                    "prefix_len": 3
                    }
                ]
            }
        }
}
```
#### CSV File Parsing
Users can specify a comma-separated values (CSV) file to filter large lists. Simply add the file parameter followed by the absolute path to the CSV file underneath a ghost_tunnel_identifier parameter or ghost_routing_identifier.
Each label in the CSV file should be separated by a new line and contain two values separated by a comma. For ghost_tunnel_identifier, the first integer is the terminal_label and the second integer is the service_label. For ghost_routing_identifier, the first integer is the value and second integer is the prefix_len.
```
{
    "delay": {
        "ghost_tunnel_identifier": {
            "file": "example/usps_api/filters/delay_test.csv",
            "ghostlabel": [
                {
                "terminal_label":15,
                "service_label": 213,
                }
            ]
        },
        "seconds": 4
    }
}
```

delay_test.csv
```
123,456
678,999
```
## Client
The [client.cc](example/usps_api/client.cc) file demonstrates how to create requests from the server. When run, the client does nothing currently. To create requests, modify the main function to call the 'CreateSfcTunnel' or 'CreateSfcRoute' function.
### Running the Client

To run the client, build the project using bazel and then run the 'run-client' binary that was generated.
Specify the host and port using abseil flags HOST and PORT.
This will look something like:
```
./bazel-bin/example/usps_api/run-client -HOST="localhost" -PORT=1234
```

--------------------------------------------------------------------------------

This is not an officially supported Google product.
