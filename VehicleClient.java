import javax.swing.*;
import javax.swing.border.TitledBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;

public class VehicleClient extends JFrame {
    // Componentes de red
    private Socket socket;
    private BufferedReader reader;
    private PrintWriter writer;
    private boolean connected = false;
    private boolean running = false;
    private String clientType = null;

    // Datos de telemetría
    private double speed = 0.0;
    private int battery = 100;
    private double temperature = 22.5;
    private String direction = "FORWARD";

    // Componentes de la interfaz
    private JTextField hostField;
    private JTextField portField;
    private JComboBox<String> clientTypeCombo;
    private JPasswordField passwordField;
    private JButton connectButton;
    private JButton disconnectButton;

    // Telemetría
    private JLabel speedLabel;
    private JLabel batteryLabel;
    private JProgressBar batteryProgress;
    private JLabel temperatureLabel;
    private JLabel directionLabel;

    // Controles de admin
    private JButton speedUpButton;
    private JButton slowDownButton;
    private JButton turnLeftButton;
    private JButton turnRightButton;
    private JButton listClientsButton;
    private JButton rechargeButton;

    // Estado
    private JLabel statusLabel;
    private JTextArea logArea;

    public VehicleClient() {
        setupGUI();
    }

    private void setupGUI() {
        setTitle("Cliente Vehículo Autónomo - Java");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());
        setResizable(false);

        // Panel principal
        JPanel mainPanel = new JPanel(new BorderLayout());
        add(mainPanel, BorderLayout.CENTER);

        // Panel de conexión
        JPanel connectionPanel = createConnectionPanel();
        mainPanel.add(connectionPanel, BorderLayout.NORTH);

        // Panel central con telemetría y controles
        JPanel centerPanel = new JPanel(new BorderLayout());

        // Panel de telemetría
        JPanel telemetryPanel = createTelemetryPanel();
        centerPanel.add(telemetryPanel, BorderLayout.NORTH);

        // Panel de controles
        JPanel controlsPanel = createControlsPanel();
        centerPanel.add(controlsPanel, BorderLayout.CENTER);

        mainPanel.add(centerPanel, BorderLayout.CENTER);

        // Panel de estado y logs
        JPanel statusPanel = createStatusPanel();
        mainPanel.add(statusPanel, BorderLayout.SOUTH);

        pack();
        setLocationRelativeTo(null);
    }

    private JPanel createConnectionPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        panel.setBorder(new TitledBorder("Conexión"));
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);

        // Host
        gbc.gridx = 0; gbc.gridy = 0;
        panel.add(new JLabel("Host:"), gbc);
        gbc.gridx = 1;
        hostField = new JTextField("localhost", 15);
        panel.add(hostField, gbc);

        // Puerto
        gbc.gridx = 2;
        panel.add(new JLabel("Puerto:"), gbc);
        gbc.gridx = 3;
        portField = new JTextField("8080", 8);
        panel.add(portField, gbc);

        // Tipo de cliente
        gbc.gridx = 0; gbc.gridy = 1;
        panel.add(new JLabel("Tipo:"), gbc);
        gbc.gridx = 1;
        clientTypeCombo = new JComboBox<>(new String[]{"OBSERVER", "ADMIN"});
        panel.add(clientTypeCombo, gbc);

        // Contraseña
        gbc.gridx = 2;
        panel.add(new JLabel("Contraseña:"), gbc);
        gbc.gridx = 3;
        passwordField = new JPasswordField(10);
        panel.add(passwordField, gbc);

        // Botones
        gbc.gridx = 0; gbc.gridy = 2;
        gbc.gridwidth = 2;
        connectButton = new JButton("Conectar");
        connectButton.addActionListener(e -> connect());
        panel.add(connectButton, gbc);

        gbc.gridx = 2;
        disconnectButton = new JButton("Desconectar");
        disconnectButton.addActionListener(e -> disconnect());
        disconnectButton.setEnabled(false);
        panel.add(disconnectButton, gbc);

        return panel;
    }

    private JPanel createTelemetryPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        panel.setBorder(new TitledBorder("Telemetría del Vehículo"));
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(10, 10, 10, 10);

        Font labelFont = new Font("Arial", Font.BOLD, 14);
        Font valueFont = new Font("Arial", Font.PLAIN, 18);

        // Velocidad
        gbc.gridx = 0; gbc.gridy = 0;
        JLabel speedTitleLabel = new JLabel("Velocidad:");
        speedTitleLabel.setFont(labelFont);
        panel.add(speedTitleLabel, gbc);
        gbc.gridx = 1;
        speedLabel = new JLabel("0.0 km/h");
        speedLabel.setFont(valueFont);
        panel.add(speedLabel, gbc);

        // Batería
        gbc.gridx = 0; gbc.gridy = 1;
        JLabel batteryTitleLabel = new JLabel("Batería:");
        batteryTitleLabel.setFont(labelFont);
        panel.add(batteryTitleLabel, gbc);
        gbc.gridx = 1;
        batteryLabel = new JLabel("100%");
        batteryLabel.setFont(valueFont);
        panel.add(batteryLabel, gbc);

        // Barra de progreso de batería
        gbc.gridx = 0; gbc.gridy = 2;
        gbc.gridwidth = 2;
        batteryProgress = new JProgressBar(0, 100);
        batteryProgress.setValue(100);
        batteryProgress.setStringPainted(true);
        batteryProgress.setPreferredSize(new Dimension(300, 25));
        panel.add(batteryProgress, gbc);

        // Temperatura
        gbc.gridx = 0; gbc.gridy = 3;
        gbc.gridwidth = 1;
        JLabel tempTitleLabel = new JLabel("Temperatura:");
        tempTitleLabel.setFont(labelFont);
        panel.add(tempTitleLabel, gbc);
        gbc.gridx = 1;
        temperatureLabel = new JLabel("22.5°C");
        temperatureLabel.setFont(valueFont);
        panel.add(temperatureLabel, gbc);

        // Dirección
        gbc.gridx = 0; gbc.gridy = 4;
        JLabel directionTitleLabel = new JLabel("Dirección:");
        directionTitleLabel.setFont(labelFont);
        panel.add(directionTitleLabel, gbc);
        gbc.gridx = 1;
        directionLabel = new JLabel("FORWARD");
        directionLabel.setFont(valueFont);
        panel.add(directionLabel, gbc);

        return panel;
    }

    private JPanel createControlsPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        panel.setBorder(new TitledBorder("Controles (Solo Administrador)"));
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);

        // Botones de control
        gbc.gridx = 0; gbc.gridy = 0;
        speedUpButton = new JButton("Acelerar");
        speedUpButton.addActionListener(e -> sendCommand("SPEED_UP"));
        speedUpButton.setEnabled(false);
        panel.add(speedUpButton, gbc);

        gbc.gridx = 1;
        slowDownButton = new JButton("Frenar");
        slowDownButton.addActionListener(e -> sendCommand("SLOW_DOWN"));
        slowDownButton.setEnabled(false);
        panel.add(slowDownButton, gbc);

        gbc.gridx = 0; gbc.gridy = 1;
        turnLeftButton = new JButton("Girar Izquierda");
        turnLeftButton.addActionListener(e -> sendCommand("TURN_LEFT"));
        turnLeftButton.setEnabled(false);
        panel.add(turnLeftButton, gbc);

        gbc.gridx = 1;
        turnRightButton = new JButton("Girar Derecha");
        turnRightButton.addActionListener(e -> sendCommand("TURN_RIGHT"));
        turnRightButton.setEnabled(false);
        panel.add(turnRightButton, gbc);

        gbc.gridx = 0; gbc.gridy = 2;
        gbc.gridwidth = 2;
        listClientsButton = new JButton("Listar Clientes");
        listClientsButton.addActionListener(e -> sendCommand("LIST_CLIENTS"));
        listClientsButton.setEnabled(false);
        panel.add(listClientsButton, gbc);

        gbc.gridx = 0; gbc.gridy = 3;
        gbc.gridwidth = 2;
        rechargeButton = new JButton("Recargar Batería");
        rechargeButton.addActionListener(e -> sendCommand("RECHARGE"));
        rechargeButton.setEnabled(false);
        panel.add(rechargeButton, gbc);

        return panel;
    }

    private JPanel createStatusPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        panel.setBorder(new TitledBorder("Estado y Logs"));

        // Estado
        statusLabel = new JLabel("Estado: Desconectado");
        statusLabel.setFont(new Font("Arial", Font.BOLD, 12));
        panel.add(statusLabel, BorderLayout.NORTH);

        // Área de logs
        logArea = new JTextArea(8, 60);
        logArea.setEditable(false);
        logArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
        JScrollPane scrollPane = new JScrollPane(logArea);
        scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        panel.add(scrollPane, BorderLayout.CENTER);

        return panel;
    }

    private void connect() {
        try {
            String host = hostField.getText().trim();
            int port = Integer.parseInt(portField.getText().trim());
            String type = (String) clientTypeCombo.getSelectedItem();
            String password = new String(passwordField.getPassword()).trim();

            // Debug: mostrar qué se está enviando
            System.out.println("DEBUG: Conectando como tipo=" + type + ", password='" + password + "'");

            // Crear socket
            socket = new Socket(host, port);
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = new PrintWriter(socket.getOutputStream(), true);

            // Solo enviar autenticación si es ADMIN
            if ("ADMIN".equals(type)) {
                String authMsg = "AUTH:ADMIN:" + password;
                System.out.println("DEBUG: Enviando mensaje: '" + authMsg + "'");
                writer.println(authMsg);

                // Esperar respuesta de autenticación
                String response = reader.readLine();
                if (response == null || !response.startsWith("AUTH:OK")) {
                    socket.close();
                    JOptionPane.showMessageDialog(this, "Error de autenticación: " + response,
                            "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }
            }

            // Conexión exitosa
            connected = true;
            running = true;
            clientType = type;

            // Actualizar UI
            SwingUtilities.invokeLater(() -> {
                connectButton.setEnabled(false);
                disconnectButton.setEnabled(true);
                statusLabel.setText("Estado: Conectado como " + type);

                if ("ADMIN".equals(type)) {
                    enableAdminControls();
                }
            });

            // Iniciar hilo de recepción
            new Thread(this::receiveMessages).start();

            logMessage("Conectado exitosamente como " + type);

        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Error de conexión: " + e.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            if (socket != null) {
                try {
                    socket.close();
                } catch (IOException ignored) {}
            }
        }
    }

    private void disconnect() {
        try {
            if (connected && writer != null) {
                writer.println("DISCONNECT:");
            }
        } catch (Exception ignored) {}

        cleanupConnection();
    }

    private void cleanupConnection() {
        running = false;
        connected = false;

        try {
            if (socket != null) socket.close();
            if (reader != null) reader.close();
            if (writer != null) writer.close();
        } catch (IOException ignored) {}

        // Actualizar UI
        SwingUtilities.invokeLater(() -> {
            connectButton.setEnabled(true);
            disconnectButton.setEnabled(false);
            statusLabel.setText("Estado: Desconectado");
            disableAdminControls();
        });

        logMessage("Desconectado del servidor");
    }

    private void enableAdminControls() {
        speedUpButton.setEnabled(true);
        slowDownButton.setEnabled(true);
        turnLeftButton.setEnabled(true);
        turnRightButton.setEnabled(true);
        listClientsButton.setEnabled(true);
        rechargeButton.setEnabled(true);
    }

    private void disableAdminControls() {
        speedUpButton.setEnabled(false);
        slowDownButton.setEnabled(false);
        turnLeftButton.setEnabled(false);
        turnRightButton.setEnabled(false);
        listClientsButton.setEnabled(false);
        rechargeButton.setEnabled(false);
    }

    private void sendCommand(String command) {
        if (connected && writer != null) {
            try {
                writer.println("CMD:" + command);
                logMessage("Comando enviado: " + command);
            } catch (Exception e) {
                logMessage("Error enviando comando: " + e.getMessage());
            }
        }
    }

    private void receiveMessages() {
        while (running && connected) {
            try {
                String message = reader.readLine();
                if (message == null) {
                    break;
                }

                processMessage(message);

            } catch (Exception e) {
                if (running) {
                    logMessage("Error recibiendo datos: " + e.getMessage());
                }
                break;
            }
        }

        SwingUtilities.invokeLater(this::cleanupConnection);
    }

    private void processMessage(String message) {
        logMessage("Recibido: " + message);

        if (message.startsWith("TELEMETRY:")) {
            processTelemetry(message.substring(10));
        } else if (message.startsWith("CMD:OK:")) {
            logMessage("Comando ejecutado: " + message.substring(7));
        } else if (message.startsWith("CMD:FAIL:")) {
            logMessage("Error en comando: " + message.substring(9));
        } else if (message.startsWith("CLIENTS:")) {
            processClientList(message.substring(8));
        } else if (message.startsWith("ERROR:")) {
            logMessage("Error del servidor: " + message.substring(6));
        }
    }

    private void processTelemetry(String data) {
        try {
            String[] parts = data.split(",");
            if (parts.length == 4) {
                speed = Double.parseDouble(parts[0]);
                battery = Integer.parseInt(parts[1]);
                temperature = Double.parseDouble(parts[2]);
                direction = parts[3];

                // Actualizar UI en hilo de Swing
                SwingUtilities.invokeLater(this::updateTelemetryDisplay);
            }
        } catch (Exception e) {
            logMessage("Error procesando telemetría: " + e.getMessage());
        }
    }

    private void processClientList(String data) {
        String[] parts = data.split(",");
        if (parts.length > 0) {
            String count = parts[0];
            StringBuilder clientInfo = new StringBuilder("Clientes conectados: " + count + "\n");

            for (int i = 1; i < parts.length; i++) {
                if (!parts[i].isEmpty()) {
                    clientInfo.append("  - ").append(parts[i]).append("\n");
                }
            }

            SwingUtilities.invokeLater(() ->
                    JOptionPane.showMessageDialog(this, clientInfo.toString(),
                            "Lista de Clientes", JOptionPane.INFORMATION_MESSAGE));
        }
    }

    private void updateTelemetryDisplay() {
        speedLabel.setText(String.format("%.1f km/h", speed));
        batteryLabel.setText(battery + "%");
        batteryProgress.setValue(battery);
        temperatureLabel.setText(String.format("%.1f°C", temperature));
        directionLabel.setText(direction);

        // Cambiar colores según valores
        if (battery < 20) {
            batteryLabel.setForeground(Color.RED);
        } else if (battery < 50) {
            batteryLabel.setForeground(Color.ORANGE);
        } else {
            batteryLabel.setForeground(Color.GREEN);
        }

        if (speed > 80) {
            speedLabel.setForeground(Color.RED);
        } else if (speed > 60) {
            speedLabel.setForeground(Color.ORANGE);
        } else {
            speedLabel.setForeground(Color.BLACK);
        }
    }

    private void logMessage(String message) {
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
        String timestamp = sdf.format(new Date());
        String logEntry = "[" + timestamp + "] " + message + "\n";

        SwingUtilities.invokeLater(() -> {
            logArea.append(logEntry);
            logArea.setCaretPosition(logArea.getDocument().getLength());
        });
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            // Usar Look and Feel por defecto - compatible con todas las versiones
            new VehicleClient().setVisible(true);
        });
    }
}