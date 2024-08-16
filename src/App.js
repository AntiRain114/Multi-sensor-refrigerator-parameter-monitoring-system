import React from 'react';
import Dashboard from './Dashboard';

const App = () => {
  return (
    <div className="App">
      <header className="bg-blue-600 text-white p-4">
        <h1 className="text-2xl font-bold">Sensor Data Visualization</h1>
      </header>
      <main className="p-4">
        <Dashboard />
      </main>
      <footer className="bg-gray-200 p-4 mt-8">
        <p className="text-center text-gray-600">© 2024 Sensor Data App</p>
      </footer>
    </div>
  );
};

export default App;