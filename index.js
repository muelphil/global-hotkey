/**
 * @module global-hotkey
 */

const hotkey_listener = require('./build/Release/hotkey_listener.node');
const V_KEY_CODES = require('./virutalKeyCodes');

const listeners = {};
let id = 1;

/**
 * Calls the listener function associated with the given listener ID.
 *
 * @param {number} listenerId - The ID of the listener to be called.
 */
function callListener(listenerId) {
    listeners[listenerId]();
}

/**
 * Adds a listener for a specific key combination.
 *
 * @param {Object} keyCombination - The key combination to listen for.
 * @param {number} keyCombination.keyCode - The key code of the key.
 * @param {boolean} [keyCombination.meta=false] - Whether the meta key is pressed.
 * @param {boolean} [keyCombination.alt=false] - Whether the alt key is pressed.
 * @param {boolean} [keyCombination.shift=false] - Whether the shift key is pressed.
 * @param {boolean} [keyCombination.ctrl=false] - Whether the ctrl key is pressed.
 * @param {Function} listener - The function to call when the key combination is pressed.
 *
 * @returns {number} The ID of the newly added listener.
 *
 * @throws {Error} Throws an error if the key combination does not include a keyCode or at least one of meta, ctrl, or alt.
 */
function addListener(keyCombination, listener) {
    if (!keyCombination.keyCode || !(keyCombination.meta || keyCombination.ctrl || keyCombination.alt)) {
        throw new Error('Key combination must feature keyCode and at least meta, ctrl, or alt!');
    }

    const fullKeyCombination = {
        meta: false,
        alt: false,
        shift: false,
        ctrl: false,
        ...keyCombination
    };

    listeners[id] = listener;
    hotkey_listener.addListener(id, fullKeyCombination);
    return id++;
}

/**
 * Removes a listener by its ID.
 *
 * @param {number} id - The ID of the listener to remove.
 */
function removeListener(id) {
    if (listeners.hasOwnProperty(id)) {
        hotkey_listener.removeListener(id);
        delete listeners[id];
    }
}

/**
 * Starts listening for global hotkey events.
 */
function startListening() {
    hotkey_listener.startThread(callListener);
}

/**
 * Stops listening for global hotkey events.
 */
function stopListening() {
    hotkey_listener.stopThread();
}

module.exports = {startListening, stopListening, addListener, removeListener, V_KEY_CODES};