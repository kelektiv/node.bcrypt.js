'use strict';

/// encapsulate a method with a node-style callback in a Promise
/// @param {object} 'this' of the encapsulated function
/// @param {function} function to be encapsulated
/// @param {Array-like} args to be passed to the called function
/// @return {Promise} a Promise encapuslaing the function
module.exports.promise = function (fn, context, args) {

    //polyfill (for 0.12)
    //avoid [].slice.call due to performance issues on v8
    //https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Functions/arguments#Description
    //var _from = Array.from || [].slice.call;

    //for node 0.12 stupidity
    var from = Array.from;

    if (!from) {
        from = function(args) {
            return [].slice.call(args);
        };
    }

    //can't do anything without Promise
    //fail silently
    if (typeof Promise === 'undefined') {
        return;
    }

    if (!Array.isArray(args)) {
        args = from(args);
    }

    if (typeof fn !== 'function') {
        return Promise.reject(new Error('fn must be a function'));
    }

    return new Promise(function(resolve, reject) {
        args.push(function(err, data) {
            if (err) {
                reject(err);
            } else {
                resolve(data);
            }
        });

        fn.apply(context, args);
    });
};

/// @param {err} the error to be thrown
module.exports.reject = function (err) {

    // silently swallow errors if Promise is not defined
    // emulating old behavior
    if (typeof Promise === 'undefined') {
        return;
    }

    return Promise.reject(err);
};
