// Stores and serves the latest sensor reading
let latest = {
  temperature: 22.0,
  humidity: 50.0,
  timestamp: Date.now()
};

export default async function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }

  if (req.method === 'POST') {
    try {
      const body = JSON.parse(req.body || '{}');
      const { temperature, humidity } = body;

      if (typeof temperature === 'number' && typeof humidity === 'number') {
        latest = {
          temperature: parseFloat(temperature.toFixed(1)),
          humidity: parseFloat(humidity.toFixed(1)),
          timestamp: Date.now()
        };
        console.log('Updated latest:', latest);
        return res.status(200).json({ success: true });
      } else {
        return res.status(400).json({ error: 'Invalid data' });
      }
    } catch (e) {
      return res.status(400).json({ error: 'JSON parse failed' });
    }
  }

  if (req.method === 'GET') {
    return res.status(200).json(latest);
  }

  res.status(405).json({ error: 'Method not allowed' });
}