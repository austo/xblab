var port = process.argv[2]

var ws = require('websocket'),
    fs = require('fs'),
    debug = require('debug')('express'),
    express = require('express'),
    http = require('http'),
    path = require('path'),
    routes = require('./server/routes'),
    util = require('./server/util/router'),
    constants = require('./public/js/constants'),
    app = express();


app.configure(function(){
    app.set('port', port); //process.env.XBLAB_PORT || 7777
    app.set('views', __dirname + '/server/views');
    app.set('view engine', 'jade');
    app.use(express.favicon('./public/img/skull-crossbones.ico'));
    app.use(express.logger('dev'));
    app.use(express.bodyParser());
    app.use(express.methodOverride());
    app.use(express.cookieParser('super*secret*secret')); //do something better here
    app.use(express.session());
    app.use(require('stylus').middleware({ src: path.join(__dirname, 'public') }));
    app.use(app.router);
    app.use(express.static(path.join(__dirname, 'public')));
});

app.configure('development', function(){
    app.use(express.errorHandler());
});

app.get('/:group', routes.index);

var server = http.createServer(app);

server.listen(app.get('port'), function(){
    console.log("xblab relay server listening on port " + app.get('port'));
});

var router = new ws.router(),
    wsServer = new ws.server({
        httpServer: server
    });

router.attachServer(wsServer);

router.mount('*', constants.chatProtocol, util.handleWsRequest);