'use strict'
const merge = require('webpack-merge')
const prodEnv = require('./prod.env')

module.exports = merge(prodEnv, {
  NODE_ENV: '"development"',
  OMPP_SRV_URL_ENV: '"http://localhost:4040"'
})
