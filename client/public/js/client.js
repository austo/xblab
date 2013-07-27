(function(ns) {

  ns.msgDiv = function(name, text) {
    return $('<div class="msg"></div>')
      .append('<span class="name">' + name + '</span>: ')
      .append('<span class="text">' + text + '</span>');
  };

  ns.Chat = {
    socket: null,
    // would need to be port 80 if using nginx
    wsUrl: document.URL.replace(/^https?:\/\//, 'ws://'),
    initialize: function() {
      var ws = new WebSocket(ns.Chat.wsUrl, xblab.chatProtocol);
      ws.onmessage = ns.Chat.handleMessage;
      ws.onclose = ns.Chat.close;
      
      // Send message on button click or enter
      $('#send').click(function() {
        ns.Chat.send();
      });            

      $('#message').keyup(function(evt) {
        if ((evt.keyCode || evt.which) == 13) {
          ns.Chat.send();
          return false;
        }
      });
      $('#password').keyup(function(evt) {
        if ((evt.keyCode || evt.which) == 13) {
          ns.Chat.login();
          $('#login').dialog('close');
          return false;
        }
      });
      ns.Chat.socket = ws;      
    },

    // Add new message to chat.
    handleMessage: function(data) {
      console.log(data);
      if (typeof data.data === 'string' && /^{/.test(data.data)){
        var res = JSON.parse(data.data);

        // TODO: May need another broadcast type - wait for others to arrive
        if (res.state)
          switch(res.state) {
            case xblab.needCred:
              $('#login').dialog('open');
              break;
            case xblab.groupEntry:
              if (res.payload) {
                ns.Chat.jqAlert(res.payload);
              }
              break;
            default: {              
              var name = res.name || 'anonymous';
              $('#messages')
                .append(ns.msgDiv(name, res.msg))
                .animate({scrollTop: $('#messages').prop('scrollHeight')}, 0);
            }
          }        
      }
    },

    jqAlert: function(msg, title) {
      $('#jqAlert').html(msg);
      $('#jqAlert').dialog({
        title: title,
        buttons: {
          'Okay': function() {
              $(this).dialog('close');
          }
        },
        autoOpen: true
      });
    },

    login: function(){
      var un = $('#username').val(),
        pw = $('#password').val();
      if (un && pw){
        var packet = JSON.stringify({
          type: 'CRED',
          username: un,
          password: pw
        });
        ns.Chat.socket.send(packet);
      }
    },

    // TODO: highlight user-posted messages in chat
    send: function() {
      var packet = JSON.stringify({
        type: 'TRANSMISSION',
        payload: $('#message').val().replace(/\n$/, '')
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