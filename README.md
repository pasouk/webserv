Status code HTTP Response
1xx indicates an informational message only
2xx indicates success of some kind
3xx redirects the client to another URL
4xx indicates an error on the client's part
5xx indicates an error on the server's part


Sumup HTTP methods
Method	    Description	                                                                                                Possible Body
GET	        Retrieve a specific resource or a collection of resources, should not affect the data/resource              	No
POST	    Perform resource-specific processing on the request content	                                                    Yes
DELETE	    Removes target resource given by a URI	                                                                        Yes
PUT	        Creates a new resource with data from message body, if resource already exist, update it with data in body	    Yes
HEAD	    Same as GET, but do not transfer the response content	                                                        No


usefull limks
https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa