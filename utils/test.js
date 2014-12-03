
var xwalk = {};
xwalk['utils'] = undefined;

xwalk.utils = require('./utils_api');

// =========================================================

function testFunc() {
    var args = xwalk.utils.validateArguments(arguments, [
            {name: 'name', type: 'string'},
            {name: 'age', type: 'unsigned long', nullable: true, optional: true},
            {name: 'granted', type: 'boolean'},
            {name: 'description', type: 'string', nullable: true, optional: true}
        ]);

    console.log('=================================================');

    console.log('Name: ' + args.name);
    console.log('Age: ' + args.age);
    console.log('Granted: ' + args.granted);
    console.log('Description: ' + args.description);
}

testFunc('Dennis Choi', null, 'sss');

