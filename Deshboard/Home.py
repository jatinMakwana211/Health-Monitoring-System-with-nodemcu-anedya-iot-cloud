import streamlit as st
import pandas as pd
import altair as alt
from streamlit_autorefresh import st_autorefresh


from utils.anedya import anedya_config, anedya_sendCommand, anedya_getValue, anedya_setValue, fetchHartbeatData, fetchTemperatureData,fetchSpO2Data

# Configuration
nodeId = "01909310-63b6-7d9f-b406-95b06acbfc89"  # get it from anedya dashboard -> project -> node
apiKey = "d8547ec1c69ba046c76532de67476a564ed9b1791c125939fe078ca035a482ca"  # anedya project apikey
st.set_page_config(page_title="Anedya IoT Dashboard", layout="wide")

st_autorefresh(interval=30000, limit=None, key="auto-refresh-handler")

# Helper function to add vertical space
def V_SPACE(lines):
    for _ in range(lines):
        st.write("&nbsp;")

# Initialize global dataframes
HartbeatData = pd.DataFrame()
temperatureData = pd.DataFrame()
spo2Data = pd.DataFrame()

def main():
    global temperatureData,HartbeatData,spo2Data
    anedya_config(NODE_ID=nodeId, API_KEY=apiKey)

    # Initialize session state if not exist
    if "LoggedIn" not in st.session_state:
        st.session_state.LoggedIn = False

    if "CurrentTemperature" not in st.session_state:
        st.session_state.CurrentTemperature = 0

    if "CurrentHartbeatData" not in st.session_state:
        st.session_state.CurrentHartbeatData = 0

    if "CurrentSpO2" not in st.session_state:
        st.session_state.CurrentSpO2 = 0

    if st.session_state.LoggedIn is False:
        drawLogin()
    else:
        HartbeatData = fetchHartbeatData()
        temperatureData = fetchTemperatureData()
        spo2Data = fetchSpO2Data()
        drawDashboard()

def drawLogin():
    cols = st.columns([1, 0.8, 1], gap='small')
    with cols[0]:
        pass
    with cols[1]:
        st.title("Anedya Demo Dashboard", anchor=False)
        username_inp = st.text_input("Username")
        password_inp = st.text_input("Password", type="password")
        submit_button = st.button(label="Submit")

        if submit_button:
            if username_inp == "jatin" and password_inp == "jatin":
                st.session_state.LoggedIn = True
                st.experimental_rerun()
            else:
                st.error("Invalid Credential!")
    with cols[2]:
        pass

def drawDashboard():
    headercols = st.columns([1, 0.1, 0.1], gap="small")
    with headercols[0]:
        st.title("Anedya Demo Dashboard", anchor=False)
    with headercols[1]:
         st.button("Refresh")

    with headercols[2]:
        logout = st.button("Logout")

    if logout:
        st.session_state.LoggedIn = False
        st.experimental_rerun()

    st.markdown("This dashboard provides live view of the Anedya's Office. Also allowing you to control the Light and Fan remotely!")

    st.subheader(body="Current Status", anchor=False)
    cols = st.columns(3, gap="medium")
    with cols[0]:
        st.metric(label="Temperature", value=f"{st.session_state.CurrentTemperature} °C")
    with cols[1]:
        st.metric(label="Hartbeat", value=f"{st.session_state.CurrentHartbeat} BPM")
    with cols[2]:
        st.metric(label="SpO2", value=f"{st.session_state.CurrentSpO2} %")

    charts = st.columns(3, gap="small")

    with charts[0]:
        st.subheader(body="Temperature", anchor=False)
        if temperatureData.empty:
            st.write("No Data !!")
        else:
            temperature_chart_an = alt.Chart(data=temperatureData).mark_area(
                line={'color': '#1fa2ff'},
                color=alt.Gradient(
                    gradient='linear',
                    stops=[alt.GradientStop(color='#1fa2ff', offset=1),
                           alt.GradientStop(color='rgba(255,255,255,0)', offset=0)],
                    x1=1,
                    x2=1,
                    y1=1,
                    y2=0,
                ),
                interpolate='monotone',
                cursor='crosshair'
            ).encode(
                x=alt.X(
                    shorthand="Datetime:T",
                    axis=alt.Axis(format="%Y-%m-%d %H:%M:%S", title="Datetime", tickCount=10, grid=True, tickMinStep=5),
                ),
                y=alt.Y(
                    "aggregate:Q",
                    scale=alt.Scale(zero=False, domain=[10, 50]),
                    axis=alt.Axis(title="Temperature (°C)", grid=True, tickCount=10),
                ),
                tooltip=[alt.Tooltip('Datetime:T', format="%Y-%m-%d %H:%M:%S", title="Time"),
                         alt.Tooltip('aggregate:Q', format="0.2f", title="Value")],
            ).properties(height=400).interactive()

            st.altair_chart(temperature_chart_an, use_container_width=True)

    with charts[1]:
        st.subheader(body="Hartbeat", anchor=False)
        if HartbeatData.empty:
            st.write("No Data Available!")
        else:
            Hartbeat_chart_an = alt.Chart(data=HartbeatData).mark_area(
                line={'color': '#1fa2ff'},
                color=alt.Gradient(
                    gradient='linear',
                    stops=[alt.GradientStop(color='#1fa2ff', offset=1),
                           alt.GradientStop(color='rgba(255,255,255,0)', offset=0)],
                    x1=1,
                    x2=1,
                    y1=1,
                    y2=0,
                ),
                interpolate='monotone',
                cursor='crosshair'
            ).encode(
                x=alt.X(
                    shorthand="Datetime:T",
                    axis=alt.Axis(format="%Y-%m-%d %H:%M:%S", title="Datetime", tickCount=10, grid=True, tickMinStep=5),
                ),
                y=alt.Y(
                    "aggregate:Q",
                    scale=alt.Scale(domain=[60,120]),
                    axis=alt.Axis(title="Hartbeat (BPM)", grid=True, tickCount=10),
                ),
                tooltip=[alt.Tooltip('Datetime:T', format="%Y-%m-%d %H:%M:%S", title="Time"),
                         alt.Tooltip('aggregate:Q', format="0.2f", title="Value")],
            ).properties(height=400).interactive()

            st.altair_chart(Hartbeat_chart_an, use_container_width=True)

    with charts[2]:
        st.subheader(body="SpO2", anchor=False)
        if spo2Data.empty:
            st.write("No Data Available!")
        else:
            spo2_chart_an = alt.Chart(data=spo2Data).mark_area(
                line={'color': '#1fa2ff'},
                color=alt.Gradient(
                    gradient='linear',
                    stops=[alt.GradientStop(color='#1fa2ff', offset=1),
                           alt.GradientStop(color='rgba(255,255,255,0)', offset=0)],
                    x1=1,
                    x2=1,
                    y1=1,
                    y2=0,
                ),
                interpolate='monotone',
                cursor='crosshair'
            ).encode(
                x=alt.X(
                    shorthand="Datetime:T",
                    axis=alt.Axis(format="%Y-%m-%d %H:%M:%S", title="Datetime", tickCount=10, grid=True, tickMinStep=5),
                ),  # T indicates temporal (time-based) data
                y=alt.Y(
                    "aggregate:Q",
                    scale=alt.Scale(domain=[80, 100]),
                    axis=alt.Axis(title="SpO2 (%)", grid=True, tickCount=10),
                ),  # Q indicates quantitative data
                tooltip=[alt.Tooltip('Datetime:T', format="%Y-%m-%d %H:%M:%S", title="Time"),
                         alt.Tooltip('aggregate:Q', format="0.2f", title="Value")],
            ).properties(height=400).interactive()

            st.altair_chart(spo2_chart_an, use_container_width=True)


if __name__ == "__main__":
    main()

