// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getAnalytics } from "firebase/analytics";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyDgfLHgW8JfyMliZxbRCzQ0uRIH-LnmUZs",
  authDomain: "smartlockersystem-1032.firebaseapp.com",
  projectId: "smartlockersystem-1032",
  storageBucket: "smartlockersystem-1032.firebasestorage.app",
  messagingSenderId: "573242600631",
  appId: "1:573242600631:web:8a2a0fb6824beac834b132",
  measurementId: "G-SEVY344KTE"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);

export { app, analytics };
