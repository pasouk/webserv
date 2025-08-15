Status code HTTP Response
1xx indicates an informational message only
2xx indicates success of some kind
3xx redirects the client to another URL
4xx indicates an error on the client's part
5xx indicates an error on the server's part



Method	    Description	                                                                                                Possible Body
GET	        Retrieve a specific resource or a collection of resources, should not affect the data/resource              	No
POST	    Perform resource-specific processing on the request content	                                                    Yes
DELETE	    Removes target resource given by a URI	                                                                        Yes
PUT	        Creates a new resource with data from message body, if resource already exist, update it with data in body	    Yes
HEAD	    Same as GET, but do not transfer the response content	                                                        No
