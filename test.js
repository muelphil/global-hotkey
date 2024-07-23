const addon = require('.');

addon.addListener({
    ctrl: false,
    meta: true,
    alt: false,
    shift: false,
    keyCode: addon.V_KEY_CODE.V
}, () => console.log('Hello from META + V'))
addon.addListener({
    ctrl: false,
    meta: true,
    alt: false,
    shift: true,
    keyCode: 'Q'
}, () => console.log('Hello from META + Shift + Q'))
// addon.removeListener(27);

console.log('Starting thread...');
addon.startListening();

setTimeout(() => {
    console.log('Stopping thread...');
    addon.stopListening();
}, 20000); // Stop after 20 seconds