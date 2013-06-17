var util = require('../modules/util'),
    hcal = require('../../node_modules/hcal/build/Debug/hcal');

exports.getWeek = function(req, res){
        res.render('scheduler', {title: 'scheduler-test'});
};

exports.getTestData = function(req, res){   
    var from = req.query.from,
        to = req.query.to;

    if (from && to){
        var toDate = util.cleanDate(to),
            fromDate = util.cleanDate(from);
        console.log(fromDate);

        hcal.getEvents(fromDate, toDate, 1, function(err, evts){                
            if (err){
                console.log(err);
            }
            else{
                var retval = [];
                var len = evts.length, i = 0;
                for (; i < len; ++i){
                    retval[i] = {
                        id: evts[i].id(),
                        text: evts[i].description(),
                        start_date: evts[i].start(),
                        end_date: evts[i].end(),
                        room_id: evts[i].roomId()
                    };
                    console.log(evts[i].description() + " - " + evts[i].start());
                }
                res.header("Access-Control-Allow-Origin", "*");
                res.header('Content-Type', 'application/json');
                res.send(JSON.stringify(retval));
            }
        });   
    }    
};

exports.insertEvent = function(request, response){
    response.header("Access-Control-Allow-Origin", "*");
    try{
        var evt_obj = request.body.event;
        var userId = request.session.currentUser.id;
        //start, end, room_id, leader_id, description, recurring, callback
        hcal.insertEvent(evt_obj.start_date, evt_obj.end_date,
            evt_obj.room_id, evt_obj.leader_id, evt_obj.text,
            evt_obj.recurring, function(err, evt){
                if (err){
                    console.log("insert event error: " + err);
                }
                else{
                    response.send({
                        message: 'Event added successfully',
                        id: evt.id() //client already has event, just send back id
                    });
                }
            });              
    }
    catch (e){
        console.log(e);
        throw ({
            message: "That's embarrassing.",
            detail: e
        });
    }
};

var processDate = function(d){
    return (d.getMonth() + 1) + '.' + d.getDate() + '.' + d.getFullYear() + ' ' + d.getHours() + ':' + d.getMinutes();
};

var packageEvents = function(evts){
    var len = evts.length, i = 0, retval = [];
    for (; i < len; ++i){
        retval[i] = new CalEvent(evts[i].id(), evts[i].description(), evts[i].start(), evts[i].end(), evts[i].roomId());
        console.log(evts[i].description() + " - " + evts[i].start());
    }
    return retval;    
}

