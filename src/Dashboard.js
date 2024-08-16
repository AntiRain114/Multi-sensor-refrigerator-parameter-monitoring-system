import React, { useState, useEffect, useCallback } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

const API_URL = 'http://43.157.46.171:3001/api/sensor-data';

const Dashboard = () => {
  const [data, setData] = useState([]);

  const fetchData = useCallback(async () => {
    try {
      const response = await fetch(API_URL);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const rawData = await response.json();
      if (rawData.length === 0) {
        console.log('No data received from API');
      } else {
        const processedData = rawData.map(item => ({
          time: new Date(item._time).getTime(),
          temperature_1: item.temperature_1,
          temperature_2: item.temperature_2,
          temperature_3: item.temperature_3,
          humidity: item.humidity
        }));
        setData(processedData);
      }
    } catch (error) {
      console.error('Error fetching data:', error.message);
    }
  }, []);

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 60000); // 每分钟更新一次数据
    return () => clearInterval(interval);
  }, [fetchData]);

  return (
    <div className="dashboard">
      <h2 className="text-xl font-bold mb-4">Sensor Data Dashboard</h2>
      <div className="chart-container" style={{ width: '100%', height: 400 }}>
        <ResponsiveContainer>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis 
              dataKey="time" 
              type="number" 
              scale="time" 
              domain={['auto', 'auto']}
              tickFormatter={(unixTime) => new Date(unixTime).toLocaleString()}
            />
            <YAxis />
            <Tooltip labelFormatter={(unixTime) => new Date(unixTime).toLocaleString()} />
            <Legend />
            <Line type="monotone" dataKey="temperature_1" stroke="#8884d8" name="Temperature 1" />
            <Line type="monotone" dataKey="temperature_2" stroke="#82ca9d" name="Temperature 2" />
            <Line type="monotone" dataKey="temperature_3" stroke="#ffc658" name="Temperature 3" />
            <Line type="monotone" dataKey="humidity" stroke="#ff7300" name="Humidity" />
          </LineChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};

export default Dashboard;
