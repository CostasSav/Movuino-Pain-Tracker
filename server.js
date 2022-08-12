console.log('Server-side code running');

const express = require('express');
const MongoClient = require('mongodb').MongoClient;
const mongoose = require('mongoose')
const router = express.Router();
const app = express();
const fetch = require('node-fetch')  //perhaps unnecessary

var bodyParser = require('body-parser'); //perhaps unnecessary
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));


// serve files from the public directory
app.use(express.static('public'));

// connect to the db and start the express server
let db;

// ***Replace the URL below with the URL for your database***
const url = 'mongodb+srv://Costas:costas9876@cluster0.x6o3g.mongodb.net/?retryWrites=true&w=majority';

MongoClient.connect(url, (err, database) => {
  if(err) {
    return console.log(err);
  }
  db = database;
  // start the express web server listening on 8080
  app.listen(8080, () => {
    console.log('listening on 8080');
    //console.log(pain)
  });
});


// serve the homepage
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});

// create MongoDB database model
var Schema = mongoose.Schema;

var SomeModelSchema = new Schema({
    intensity: Number,
    date: Date
});

var BraceletGraph = mongoose.model('BraceletGraph', SomeModelSchema);

//receive pain data and add to MongoDB Database
app.post("/", (req, res) => {
  const bodyData = req.body;
  console.log("Received:")
  console.log(bodyData);

  BraceletGraph.create(bodyData, function (err, painData){
    if (err) return console.log(err);
  })

  module.exports = BraceletGraph;
})

app.use("/", router);

