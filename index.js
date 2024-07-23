var hotkey_listener = require('./build/Release/hotkey_listener.node');


const listeners = {};

let id = 1;

function callListener(listenerId) {
    console.log('global-hotkey::callListener', listenerId)
    const listener = listeners[listenerId];
    if (listener) {
        listener();
    }
}

function addListener(keyCombination, listener) {
    console.log('global-hotkey::addListener', keyCombination)
    if (!keyCombination.keyCode || !(keyCombination.meta || keyCombination.ctrl || keyCombination.alt)) {
        throw 'key combination must feature keyCode and at least meta, ctrl or alt!'
    }
    const fullKeyCombination = {
        meta: false,
        alt: false,
        shift: false,
        ctrl: false,
        ...keyCombination
    }
    listeners[id] = listener;
    hotkey_listener.addListener(id, fullKeyCombination)
    return id++;
}

function removeListener(id) {
    console.log('global-hotkey::removeListener', id)
    if (listeners[id]) {
        listeners[id] = undefined;
        hotkey_listener.removeListener(id);
    }
}

function startListening() {
    console.log('global-hotkey::startListening')
    hotkey_listener.startThread(callListener);
}

function stopListening() {
    console.log('global-hotkey::stopListening')
    hotkey_listener.stopThread();
}

module.exports = {startListening, stopListening, addListener, removeListener};
