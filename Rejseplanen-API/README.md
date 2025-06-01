# 🚀 README for Rejseplanen API Project

Welcome to our GitHub repository! This project demonstrates how to build a custom frontend for Danish public transportation—similar to the official Rejseplanen app—by leveraging the **Rejseplanen API**. This README is designed for non-experts: it explains what an API is, how the Rejseplanen API works, and how you can use it in your own projects.

---

## 📘 1. What Is an API?

An **API (Application Programming Interface)** is a set of rules and protocols that allows one software application to communicate with another. Think of it like a waiter at a restaurant:

1. **Request**: You (the client) tell the waiter (the API) what you want.
2. **Processing**: The waiter relays your request to the kitchen (the server).
3. **Response**: The kitchen prepares the meal (the data) and the waiter delivers it back to you.

In web development, APIs let you request data or services from a remote server. For example, a weather app uses a weather API to fetch the current forecast. In our case, the Rejseplanen API provides up-to-date transport information.

> ❗️ **Why APIs Matter**: They enable developers to build applications without having to host and maintain large datasets themselves. Instead, you can rely on an existing service (like Rejseplanen) that continuously updates and maintains accurate information.

---

## 🧭 2. About the Rejseplanen API

The **Rejseplanen API** (version 2.0) is a RESTful interface provided by [Rejseplanen Labs](https://labs.rejseplanen.dk/hc/da/articles/21554723926557-Om-API-2-0 "Om API 2.0") for accessing Denmark’s public transport data, including:

* **Trip Planning**: Calculate journeys between two points (addresses, station IDs, or coordinates).
* **Departure & Arrival Boards**: View next departures or arrivals at a specific stop.
* **Fare & Tariff Information**: Retrieve prices for various ticket types (Single Ticket, Rejsekort, Season Pass, etc.).
* **Location Services**: Search for stations or addresses, find nearby stops, and get station details.
* **Zone & Tariff Maps**: Convert zones or retrieve geometric data for fare zones.

The official [Rejseplanen web app](https://www.rejseplanen.dk/ "Rejseplanen Web Site") and mobile app are essentially frontends that consume this API. This means anyone can build a similar app tailored to their own needs.

---

## 🔑 3. Accessing the Rejseplanen API

To start using the Rejseplanen API, you need an **API key** (accessId). Follow these steps:

1. **Request Access**

   * Visit the [Rejseplanen Labs Access Page](https://labs.rejseplanen.dk/hc/da/articles/21553113674909-Adgang-til-data-fra-Labs "Adgang til Data fra Labs").
   * Fill out the form to request API access (mention your project, intended usage, and whether it’s commercial or non-commercial).

2. **Approval & Credentials**

   * After approval, you receive login credentials for Rejseplanen Labs.
   * Log in and navigate to “My Applications” to copy your `accessId` (API key).

3. **Usage Tiers & Limits**

   * **Non-commercial use**: Free up to **50,000 requests/month**.
   * **Commercial use**:

     * **100,000 req/day**: €5,000/year + service fee.
     * **200,000 req/day**: €8,500/year + service fee.
   * For custom needs, contact Rejseplanen Labs to discuss tailored plans.

---

## 🔍 4. Core API Services (v2.0)

Below is a summary of the most commonly used endpoints. For complete documentation, see the official [API 2.0 Overview](https://labs.rejseplanen.dk/hc/da/articles/21554723926557-Om-API-2-0 "Om API 2.0").

### 4.1. Location Services

* **Address Lookup**: Search for addresses near a coordinate.

  * Endpoint: `/location.name`
  * Example: `GET https://api.rejseplanen.dk/location.name?accessId=YOUR_ACCESS_ID&input=S%C3%B8lvgade%204,%20Copenhagen`

* **Nearby Stops**: Find stops within 1 km of a coordinate.

  * Endpoint: `/location.nearbystops`
  * Example: `GET https://api.rejseplanen.dk/location.nearbystops?accessId=YOUR_ACCESS_ID&latitude=55.678&longitude=12.577&maxNo=10`

* **Location Details**: Retrieve detailed info for a given location ID.

  * Endpoint: `/location.name?accessId=YOUR_ACCESS_ID&id=8600512`
  * Response includes station name, coordinates, and available lines.

### 4.2. Trip Planning

* **Trip Search**: Plan a trip between an origin and a destination.

  * Endpoint: `/trip`

  * Basic Parameters:

    * `originExtId`: Station ID or coordinate for origin.
    * `destExtId`: Station ID or coordinate for destination.
    * `date` & `time`: Desired departure or arrival.
    * `format`: `json` or `xml`.

  * **Example (JSON, no Tariff)**:

    ```bash
    curl \
      "https://api.rejseplanen.dk/trip?accessId=YOUR_ACCESS_ID&originExtId=8600626&destExtId=8600512&date=2023-06-01&time=08:00&format=json"
    ```

  * **Enhanced Trip with Fare Data**:

    * Add `tariff=1` and `travellerProfileData` to include fare calculations.
    * `travellerProfileData` is a URL-encoded JSON string specifying tariff types and passenger categories.

    ```bash
    curl \
      "https://api.rejseplanen.dk/trip?accessId=YOUR_ACCESS_ID&tariff=1&travellerProfileData=%7B%22trf%22%3A%5B%22RK%22%2C%22ST%22%5D%2C%22psg%22%3A%5B%22A%22%2C%22C%22%5D%7D&originExtId=8600626&destExtId=8600512&date=2023-06-01&time=08:00&format=json"
    ```

  * **travellerProfileData Structure** (JSON before encoding):

    ```json
    {
      "trf": ["RK", "ST"],        // Tariff types: "RK"=Rejsekort, "ST"=Single Ticket, etc.
      "psg": ["A", "C"]           // Passenger types: "A"=Adult, "C"=Child, etc.
    }
    ```

  * **Response**: Contains `TariffResult` object with `fareSetItems` and `fareItems` detailing all fare options.

### 4.3. Departure & Arrival Boards

* **Departure Board (Departure)**:

  * Endpoint: `/departureBoard`
  * Parameters: `id` (station ID), `date`, `time`, `format`.
  * Example:

    ```bash
    curl \
      "https://api.rejseplanen.dk/departureBoard?accessId=YOUR_ACCESS_ID&id=8600512&date=2023-06-01&time=09:00&format=json"
    ```

* **Arrival Board (Arrival)**:

  * Endpoint: `/arrivalBoard`
  * Similar parameters to departure board.

### 4.4. Fare & Tariff Information

#### 4.4.1. spPrice (Price Search)

* **spPrice**: Get fare options for a journey (origin and destination as addresses or station names).

  * Endpoint: `/spPrice`

  * Parameters:

    * `oaddr`: Origin address.
    * `daddr`: Destination address.
    * `psgs`: Passenger types (e.g., `["A","C"]`).
    * `issue_on`: Date of travel (YYYY-MM-DD).
    * `journey_info`: `1` to include trip details in response.
    * `format`: `json` or `xml`.

  * **Example**:

    ```bash
    curl \
      "https://api.rejseplanen.dk/spPrice?accessId=YOUR_ACCESS_ID&format=json&oaddr=Odense+St.&daddr=Ringstedt+St.&psgs=%5B%22A%22%2C%22C%22%5D&issue_on=2023-06-01&journey_info=1"
    ```

#### 4.4.2. spPriceReconstruction (Reconstruct Trip + Fare)

* Allows fare calculation when you have a previous trip context (`ctxRecons`).

  * Endpoint: `/spPriceReconstruction`

  * Parameters:

    * `ctxRecons`: Reconstruction context string returned from a prior `/trip` call.
    * `psgs`: Passenger types.
    * `issue_on`: Travel date.
    * Other flags like `with_purse`.

  * **Example**:

    ```bash
    curl \
      "https://api.rejseplanen.dk/spPriceReconstruction?accessId=YOUR_ACCESS_ID&format=json&psgs=%5B%22A%22%2C%22C%22%2C%22P%22%5D&issue_on=2023-06-01&with_purse=false&journey_info=0&valid_from=2023-06-01&ctxRecons=T%2420230601..."
    ```

#### 4.4.3. Zone & Tariff Utilities

* `convertZones`: Convert between different zoning systems.

  * Example: `GET https://api.rejseplanen.dk/convertZones?accessId=YOUR_ACCESS_ID&if=4&of=8&z=1001,1002,1003&o=1002&d=1004&uc=self&format=json`
* `spPriceForZones`: Get price for specific fare zones.

  * Example: `GET https://api.rejseplanen.dk/spPriceForZones?accessId=YOUR_ACCESS_ID&zn=5&fs=7&format=json`
* `spDailyZonalPrices`: Daily prices for a zone.

  * Example: `GET https://api.rejseplanen.dk/spDailyZonalPrices?accessId=YOUR_ACCESS_ID&fs=7&format=json`

### 4.5. Additional Services

* **stRoutes**: Public transport routing between two stations.

  * Example:

    ```bash
    curl \
      "https://api.rejseplanen.dk/stRoutes?accessId=YOUR_ACCESS_ID&oStop=8600512&dStop=8600611&psgs=%5B%22A%22%2C%22C%22%5D&travel_on=2023-06-01&format=json"
    ```
* **stRoutesAddon**: Only origin, no destination (Zealand only).

  * Example:

    ```bash
    curl \
      "https://api.rejseplanen.dk/stRoutesAddon?accessId=YOUR_ACCESS_ID&oStop=8600611&psgs=%5B%22A%22%2C%22C%22%5D&mode=DSB-2&format=json"
    ```
* **PolygonForZone** (HAITI-based): Retrieve a polygon for a fare zone.

  * Example:

    ```bash
    curl \
      "https://hkrp.hafas.cloud/bin/haapi.exe?style=dsbtarif&reduce=1&getcontent=1041&date=2023-06-01"
    ```

---

## 🔧 5. Code Snippets & Examples

Below are some practical examples showing how to integrate the Rejseplanen API into your code.

### 5.1. JavaScript (Fetch)

```js
// Example: Plan a simple trip without fare information
async function fetchTrip() {
  const accessId = "YOUR_ACCESS_ID";
  const url = new URL("https://api.rejseplanen.dk/trip");
  const params = {
    accessId,
    originExtId: 8600626,       // København H (Station ID)
    destExtId: 8600512,         // Nørreport St. (Station ID)
    date: "2023-06-01",
    time: "08:00",
    format: "json"
  };

  Object.keys(params).forEach(key => url.searchParams.append(key, params[key]));

  const response = await fetch(url);
  const data = await response.json();
  console.log("Trip Data:", data);
}

// Example: Plan a trip with fare calculation (Rejsekort + Single Ticket)
async function fetchTripWithFare() {
  const accessId = "YOUR_ACCESS_ID";
  const travellerProfileData = JSON.stringify({ trf: ["RK", "ST"], psg: ["A", "C"] });
  const encodedProfile = encodeURIComponent(travellerProfileData);

  const url = new URL("https://api.rejseplanen.dk/trip");
  const params = {
    accessId,
    tariff: 1,
    travellerProfileData: encodedProfile,
    originExtId: 8600626,
    destExtId: 8600512,
    date: "2023-06-01",
    time: "08:00",
    format: "json"
  };

  Object.keys(params).forEach(key => url.searchParams.append(key, params[key]));

  const response = await fetch(url);
  const data = await response.json();
  console.log("Trip with Fare Data:", data);
}
```

### 5.2. cURL Examples

```bash
# 1. Simple trip search (JSON)
curl "https://api.rejseplanen.dk/trip?accessId=YOUR_ACCESS_ID&originExtId=8600626&destExtId=8600512&date=2023-06-01&time=08:00&format=json"

# 2. Trip search with fare calculation (Rejsekort + Single Ticket)
curl "https://api.rejseplanen.dk/trip?accessId=YOUR_ACCESS_ID&tariff=1&travellerProfileData=%7B%22trf%22%3A%5B%22RK%22%2C%22ST%22%5D%2C%22psg%22%3A%5B%22A%22%2C%22C%22%5D%7D&originExtId=8600626&destExtId=8600512&date=2023-06-01&time=08:00&format=json"

# 3. Departure board (JSON)
curl "https://api.rejseplanen.dk/departureBoard?accessId=YOUR_ACCESS_ID&id=8600512&date=2023-06-01&time=09:00&format=json"

# 4. spPrice (fare search by address)
curl "https://api.rejseplanen.dk/spPrice?accessId=YOUR_ACCESS_ID&format=json&oaddr=Odense+St.&daddr=Ringstedt+St.&psgs=%5B%22A%22%2C%22C%22%5D&issue_on=2023-06-01&journey_info=1"
```

---

## 🌐 6. Helpful Links

* [Rejseplanen Labs Access & API Documentation](https://labs.rejseplanen.dk/hc/da/articles/21554723926557-Om-API-2-0 "Rejseplanen API 2.0 Overview")
* [How to Get Access to Rejseplanen Data](https://labs.rejseplanen.dk/hc/da/articles/21553113674909-Adgang-til-data-fra-Labs "Adgang til Data fra Labs")
* [Official Rejseplanen Website](https://www.rejseplanen.dk/ "Rejseplanen Web Site")
* [Hacon HAFAS API Documentation](https://www.hacon.de/zugangsbasierte-dokumentation/ "HAFAS API Overview")

---

## 🤝 7. Contributing

We welcome contributions! If you find issues or have ideas for improvements:

1. Fork this repository.
2. Create a new branch (`git checkout -b feature/your-feature`).
3. Commit your changes (`git commit -m "Add new feature"`).
4. Push to your branch (`git push origin feature/your-feature`).
5. Open a Pull Request describing your changes.

Please ensure your code follows best practices and includes relevant comments.

---

## 📄 8. License

This project is licensed under the [MIT License](LICENSE "MIT License"). Feel free to use, modify, and distribute.

---

## 🙌 9. Acknowledgments

* Thanks to [Rejseplanen Labs](https://labs.rejseplanen.dk/ "Rejseplanen Labs") for providing free access to public transport data.
* Original fare migration documentation by HaCon Ingenieurgesellschaft.

Happy coding! 🎉
