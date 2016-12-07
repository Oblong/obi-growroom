// Clean up the scene graph by removing all Growroom nodes
function CleanUp (){
    greenhouse.Origin.Instance().ClearScene();
}

// global.has_run controls wether the first time initialization
// code will run. This needs to be a global to prevent re-evaluation
// of this code from resetting the value.
console.log("Initialization has run: " + global.has_run);

// Declare but do not init libraries, these should only be initialized
// on the first run.

var l; // lodash
var j; // javlin-pure
var perceive;
var accelerate;
var inertia;

if (!global.has_run){
    console.log("Doing first run initialization");
    l = require('lodash');
    j = require('javlin-pure');
    perceive = require('perceive');
    accelerate = require('pocket-physics/accelerate2d');
    inertia = require('pocket-physics/inertia2d');

    global.libs = {
        l: l,
        j: j,
        perceive: perceive,
        accelerate: accelerate,
        inertia: inertia
    };
    global.has_run = true;
} else {
    console.log("Re-run, cleaning scene graph");
    CleanUp ();
    l = global.libs.l;
    j = global.libs.j;
    perceive = global.libs.perceive;
    accelerate = global.libs.accelerate;
    inertia = global.libs.inertia;
}

staging.SetFeldsColor([0.44, 0.59, 0.56]);
