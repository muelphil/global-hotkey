const addon = require('.');

function callback(data) {
    console.log('Received from thread:', data);
}

// Create an array of objects to pass to the native addon
const objects = [
    {message: 'Object 1'},
    {message: 'Object 2'},
    {message: 'Object 3'}
];
// addon.printOnShortcut();

addon.addListener({
    ctrl: false,
    meta: true,
    alt: false,
    shift: false,
    keyCode: 'V'
}, () => console.log('Hello from META + V'))
addon.addListener({
    ctrl: false,
    meta: true,
    alt: false,
    shift: true,
    keyCode: 'Q'
}, () => console.log('Hello from META + Shift + V'))
// addon.removeListener(27);


console.log('Starting thread with objects...');
addon.startListening();

setTimeout(() => {
    console.log('Stopping thread...');
    addon.stopListening();
}, 20000); // Stop after 20 seconds