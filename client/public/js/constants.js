(function(exports){
  exports.chatProtocol = 'xblab-chat-protocol';
  exports.needCred = 'NEEDCRED';
  exports.groupEntry = 'GROUPENTRY';

// if exports is undefined, we're in the browser...
// define NS if necessary & invoke with that instead
}(typeof exports === 'undefined' ? this['xblab'] = this['xblab'] || {} : exports));