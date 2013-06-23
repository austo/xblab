(function(ns) {

    ns.Chat = {
        socket: null,
        wsUrl: document.URL.replace(/^https?:\/\//, 'ws://'), //port 80 when using nginx
        initialize: function() {
            var ws = new WebSocket(ns.Chat.wsUrl, xblab.chatProtocol);
            ws.onmessage = ns.Chat.handleMessage;
            ws.onclose = ns.Chat.close;
            
            //Send message on button click or enter
            $('#send').click(function() {
                ns.Chat.send();
            });            

            $('#message').keyup(function(evt) {
                if ((evt.keyCode || evt.which) == 13) {
                    ns.Chat.send();
                    return false;
                }
            });
            ns.Chat.socket = ws;      
        },

        //Add new message to chat.
        handleMessage: function(data) {
            console.log(data);
            if (typeof data.data === 'string' && /^{/.test(data.data)){
                var res = JSON.parse(data.data);

                // May need another dialog permutation - wait for others to arrive
                if (res.state && res.state === 'NEEDCRED'){
                    $('#login').dialog('open');
                }
                else{
                    var name = res.name || 'anonymous';
                    var msg = $('<div class="msg"></div>')
                        .append('<span class="name">' + name + '</span>: ')
                        .append('<span class="text">' + res.msg + '</span>');

                    $('#messages')
                        .append(msg)
                        .animate({scrollTop: $('#messages').prop('scrollHeight')}, 0);
                    }
            }
        },

        //TODO: highlight user-posted messages in chat
        send: function() {
            var packet = JSON.stringify({
                name: $('#name').val(),
                msg: $('#message').val().replace(/\n$/, '')
            });

            ns.Chat.socket.send(packet);
            $('#message').val('');
            return false;
        },        

        close: function(){
            console.log("WebSocket Connection Closed.");
        }
    };

}(xblab = xblab || {}));