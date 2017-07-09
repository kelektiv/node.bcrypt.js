#!/usr/bin/env node
var bcrypt = require('./bcrypt');

function range(start, stop, fn) {
    var i, array = [];
    for (i = start; i <= stop; i++) {
        array.push(fn(i));
    }
    return array;
}

function sum(values) {
    return values.reduce(function (a, b) { return a + b; });
}

function randomstring(length) {
    var chars = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    return sum(range(1, length, function () {
        return chars[Math.floor(Math.random() * chars.length)];
    }));
}

function time_compare(rounds) {
    var password = randomstring(25);
    var start = process.hrtime();
    bcrypt.hashSync(password, rounds);
    var elapsed = process.hrtime(start);
    return (elapsed[0] * 1e9 + elapsed[1]) / 1e6;
}

range(10, 16, function (rounds) {
    process.stdout.write(rounds + ' rounds: ');
    var values = range(1, 10, function (i) {
        var time = time_compare(rounds);
        process.stdout.write(time.toFixed(3));
        if (i != 10) process.stdout.write(', ');
        return time;
    });
    var avg = sum(values) / values.length;
    process.stdout.write(' = ' + avg.toFixed(3) + 'ms')
    process.stdout.write(' ('+(1000/avg).toFixed(2)+' hash/sec)\n');
});
