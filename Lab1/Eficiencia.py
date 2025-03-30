import pandas as pd
import matplotlib.pyplot as plt

# Crear datos de ejemplo
data = {
    'X': [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17],
    'Y': [0, 0, 0, 0, 0, 0, 1, 3, 15, 89, 409, 2619, 13513, 87300, 592624, 33674360]
}

# Crear un DataFrame
df = pd.DataFrame(data)

# Mostrar la tabla
print("Tabla de datos:")
print(df)

# Crear una gráfica de líneas
plt.plot(df['X'], df['Y'], marker='o', color='skyblue', label='Datos')
plt.title('Gráfica de Datos')
plt.xlabel('X')
plt.ylabel('Y')
plt.yscale('log')  # Escala logarítmica para manejar valores grandes
plt.grid(axis='both', linestyle='--', alpha=0.7)
plt.legend()

# Mostrar la gráfica
for i, txt in enumerate(df['Y']):
    plt.text(df['X'][i], df['Y'][i], str(txt), fontsize=8, ha='right', va='bottom')

plt.show()