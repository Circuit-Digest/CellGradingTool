# [CellGradingTool](https://circuitdigest.com/videos/diy-modular-cell-grading-machine)
<img src="https://github.com/jobitjoseph/CellGradingTool/blob/26f4b29b56a88dff072981b01a6583991b79fa38/Images/Title%20Image.png" width="" alt="alt_text" title="image_tooltip">
<br>
Testing lithium-ion cells is critical for ensuring their performance, safety, and longevity, particularly in applications such as electric vehicles, energy storage systems, and portable electronics. As these cells can degrade over time, accurate grading tools are necessary to assess their charging, discharging, and overall capacity characteristics. This DIY modular cell grading tool addresses this need by providing a streamlined and scalable solution for testing multiple lithium-ion cells simultaneously while offering precise control over charging and discharging cycles. The tool’s modularity allows users to grade up to 250 cells in parallel, making it a versatile choice for both small and large-scale testing environments. At the heart of the tool is the STM32 G071KBU3 microcontroller, powered by an ADP7118 3.3V LDO regulator. The tool is designed to detect the voltage of each inserted cell and display it on a two-digit seven-segment display, providing instant feedback. When connected to the companion software, users can initiate a test, charging the cell at a constant current of 1A using the TP4056 IC. Upon reaching full charge, the system will disable the charging circuitry and transition into discharge mode, where the discharge current and voltage are monitored through the high-precision INA236 sensor. This setup ensures accurate tracking of both charging and discharging parameters, essential for detailed cell grading.
<br>
Communication between the tool and the PC is managed via the RS485 protocol, utilising the SP485REN transceiver IC. This allows us to connect multiple tools in parallel, with each module being independently controlled by the companion app. The discharge circuit uses an IRF540 MOSFET as a programable DC load and a current control circuit built around the MCP6486 op-amp, ensuring precise current regulation. The DAC output from the STM32G071 is used for adjusting the discharge current, allowing for highly accurate measurements of cell performance under load conditions. The companion software, written in Processing, provides a user-friendly interface for controlling up to 8 modules, with the flexibility to expand to 250 modules. Users can set parameters such as discharge current and voltage limits for each module independently and start tests with a single button click. The software also includes a feature for graphing the discharge voltage curve, providing valuable insights into the cell’s behaviour over time. Even if the cell is removed, the tool retains the test data, ensuring no loss of critical information. With this tool, users can efficiently grade lithium-ion cells and make informed decisions about their suitability for various applications.
<img src="https://github.com/jobitjoseph/CellGradingTool/blob/81d42b987690ba5c8a1603dc29dffd6d562c6ca3/Images/CellGrading_PCB_Bottom.png" width="" alt="alt_text" title="image_tooltip">
<img src="https://github.com/jobitjoseph/CellGradingTool/blob/81d42b987690ba5c8a1603dc29dffd6d562c6ca3/Images/CellGrading_PCB_TOP.png" width="" alt="alt_text" title="image_tooltip">
<img src="https://github.com/jobitjoseph/CellGradingTool/blob/81d42b987690ba5c8a1603dc29dffd6d562c6ca3/Images/CellGradingControlApp.png" width="" alt="alt_text" title="image_tooltip">
