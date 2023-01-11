const path = require('path');
const webpack = require('webpack');
const HtmlWebpackPlugin = require('html-webpack-plugin');

let devServerOptions =
{
   // Tell the server where to serve the content from. This is only necessary if
   // you want to serve static files. static.publicPath will be used to determine
   // where the bundles should be served from and takes precedence.
   static: [{ directory: 'dist',
              // Tell dev-server to watch the files served by the static.directory
              // option. It is enabled by default, and file changes will trigger a
              // full page reload.
              watch: true,
            }
           ],

   // Enable gzip compression for everything served:
   compress: false,

   // Specify a port number to listen for requests on.
   port: 9000,

   // Specify a host to use. If you want your server to be
   // accessible externally, specify it like this:
   // Default is 'localhost'.
   // host: '192.168.2.95',

   // This option allows you to whitelist services that are
   // allowed to access the dev server.
   allowedHosts: [
      '192.168.2.*',
      '192.168.2.',
   ],

   // By default, the dev-server will reload/refresh the page when file changes are
   // detected.
   // devServer.hot option must be disabled or devServer.watchFiles option must be
   // enabled in order for liveReload to take effect.
   liveReload: true,

   // Enable webpack's Hot Module Replacement feature:
   // Note: requires a lot of other stuff too in index.html ...
   hot: false,

   onListening: function(devServer) {
      if (!devServer) {
        throw new Error('webpack-dev-server is not defined');
      }
      const port = devServer.server.address().port;
      console.log('Listening on port:', port);
   },

   // Tells dev-server to open the browser after server had been started.
   //open: true,
   open: ['UnScramble.html'],
};

module.exports = (env, argv) =>
{
   console.log("Processing webpack build");

   // The default 'mode' to use.
   Mode='production';

   if ( argv )
   {
      if ( argv.mode )
         if (argv.mode === 'development' )
            // Since this is processed as a command line option
            // (because of argv...) the mode must be set so that it
            // can be returned as part of the big config entry.
            Mode='development';
         else if ( argv.mode !== 'production' )
            throw new error('unhandled mode:' + argv.mode);

      if ( argv.open )
      {
         devServerOptions.open = argv.open
         console.log("telling devServer to open:" + devServerOptions.open);
      }
   }

   return {
      // Since this is processed as a command line option (because of argv...)
      // the mode must be set so that it can be returned as part of the big
      // config entry.
      mode: Mode,

      entry: './src/js/Unscramble.js',
      output: {
         path: path.resolve(__dirname, './dist'),
         filename: 'js/UnScramble_bundle.js',
         publicPath: '/',
         /// At this time the Grid class is the only thing that works
         // and since it extends Sprite, it cannot be a Library.
         // pixi-ui would be a library and it will be created as such
         // with another entry point. so for now, leave these commented out.
         // The library name means you would access it via makercam5.Grid.
         //library: 'makercam5',
         //libraryTarget: 'umd',
         //umdNamedDefine: true,

         // Turn off pathInfo, incrasing build time
         pathinfo: false,
      },
      // For when --watch is specified, automatically compile ...
      watchOptions: {

         // Add a delay before rebuilding once the first file changed.
         aggregateTimeout: 1000,

         // Check for changes every 3 seconds
         poll: 3000,
         ignored: ["/node_modules/"
                  ]
      },

      devServer: devServerOptions,

      plugins: [
         // Too messy, deal with later
         // new TSLintPlugin({
         //    files: ['./app/partKart/**/*.ts',
         //            './app/src/*.ts'
         //           ]
         // }),

         // Add a plugins section to your webpack.config.js to let
         // know Webpack that the global PIXI variable make reference
         // to pixi.js module. For instance:
         //z new webpack.ProvidePlugin({
         //z    PIXI: 'pixi.js'
         //z }),

         // The plugin will generate an HTML5 file for you that includes
         // all your webpack bundles in the body using script tags.
         new HtmlWebpackPlugin({

            // The file to write the HTML to. Defaults to index.html.
            // It will be placed in path: above.  You can add further
            // subdirectories if need be
            filename: './UnScramble.html',

            // The title to use in the generated HTML document
            title: 'UnScramble',

            //options: {
               // Keys in the template file that get replaced into the html file
               // <%= htmlWebpackPlugin.options.title %>
            //   title: 'UnScramble',
            //},

            // Instead of the default template that would be created
            // Use ours.
            template: 'src/UnScramble.html.template',

            // Allows to overwrite the parameters used in the template
            // templateParameters: {Boolean|Pbjects|Function}

            // Inject all assets of template at position
            // inject: true || 'head' || 'body' || false
            //X inject: 'body',

            // Adds the given favicon path to the output HTML
            // When you touch the URL you will see the bunny
            //favicon: 'bunny.png',

            // Inject the following meta tags
            //X meta: {
            //X    VIEWPORT: 'WIDTH=device-width, INITIAL-SCALE=1, SHRINK-TO-FIT=no'
            //X },

            // Inject a base tag.
            // E.g. base: "https://example.com/path/page.html
            //X base: 'http://localhost/~zarf',

            // Controls if and in what ways the output should be minified.
            // Default is only true for production mode
            minify: false,

            // Emit the file only if it was changed. Default is true.
            cache: true,

            // Errors details will be written into the HTML page.
            // Default is true.
            showErrors: true,

           // Allows you to add only some chunks (See docs)
           // (e.g only the unit-test chunks)
           // chunks:
           // chunksSortMode:
           // excludeChunks:

           // true render the link tags as self-closing (XHTML compliant)
           xhtml: false

         })
      ],
      //target: 'umd',
      //target: 'web',
      //z externals: Externals,
      //z resolve: {
         // Add '.ts' as resolvable extensions.
      //z    extensions: [".js"],
      //z    alias: {
      //z       $: "jquery/src/jquery",
      //z    }
      //z },
      //optimization: {
      //   minimizer: [
      //      new UglifyJsPlugin({
      //         parallel: true,
      //         uglifyOptions: {
      //            mangle: false
      //         }
      //      })
      //   ]
      //},

      // Seperate source maps
      devtool: "source-map",

      module: {
         rules: [
            {
               test: /\.js$/,
               include: [path.resolve(__dirname, "src/js")],
               exclude: [
                  // This would be a regular expression
                  /node_modules/,
                  // This would be a path
                  // Omit stuff to be worked on
               ],
               use: {
                  loader: 'babel-loader',
                  // options: {
                        // "presets": [
                            // ['es2015', {modules: true}]
                            // ['@babel/preset-env', {modules: false}]
                            //['@babel/preset-env', { targets: { esmodules: false }
                                                  //  , modules: false
                            //                      }
                            //]
                        //],
                  //   }
                  // try options: {
                  // try   strictMode: false,
                  // try }
                  // Note. Do not put other compile options here !!!
                  // ts-loader or babel-loader may override them.
                  // i.e ts-loader listFiles: true or
                  //     ts-loader outdir: 'js'
               }
            }
         ]
      },
   };
};
