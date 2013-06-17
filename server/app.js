// Load the TCP Library
var net = require('net'),
    util = require('util'),
    fs = require('fs'),
    ProtoSchema = require('protobuf').Schema,
    schema = new ProtoSchema(fs.readFileSync('src/protobuf/xblab.desc')),
    bc = schema['xblab.Broadcast'],
    trans = schema['xblab.Transmission'];


 
// Keep track of the chat clients
var clients = [];
 
// Start a TCP Server
net.createServer(function (socket) { 
 
    // Identify this client
    socket.name = socket.remoteAddress + ":" + socket.remotePort 
    //console.log(util.inspect(socket, { colors: true, depth: null }));
    // Put this new client in the list
    clients.push(socket);
     
    // Send NEEDCRED - start timeout? (2 min)
    //var ds = bcData.serialize({type: 0, nonce: 'my_fantastic_nonce'});
    var serialized = bc.serialize({signature: 'signature', data: {type: 0, nonce: 'my_fantastic_nonce'}});
    console.log(serialized);
    var next = bc.parse(serialized);
    console.log(next);
    socket.write(JSON.stringify(next));
 

    /*
        This should be a 100% binary message.
        We don't know how to parse it at this level,
        so send it down to C++ land
    */
    socket.on('data', function (data) {
        // send to xblab
        
    });
 
    // Remove the client from the list when it leaves
    socket.on('end', function () {
        clients.splice(clients.indexOf(socket), 1);
        broadcast(socket.name + " left the chat.\n");
    });
    
    // Send a message to all clients
    function broadcast(message, sender) {
        clients.forEach(function (client) {
            // Don't want to send it to sender
            if (client === sender) return;
            client.write(message);
            console.log(util.inspect(client, {colors: true, depth: null}));
        });
        // Log it to the server output too
        process.stdout.write(message)
    }
 
}).listen(5000);
 
// Put a friendly message on the terminal of the server.
console.log("Chat server running at port 5000\n");