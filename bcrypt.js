try {
  module.exports = require('./build/default/bcrypt_lib');
} catch(e) {
  //update for v0.5.5+
  module.exports = require('./build/Release/bcrypt_lib');
}
