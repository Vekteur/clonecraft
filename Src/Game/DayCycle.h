#include "Maths/GlmCommon.h"
#include "Maths/Converter.h"
#include "View/Window.h"

class DayCycle {
public:
    void update(sf::Time dt) {
        // Advance the day/night cycle, dimming both the terrain light (dayFactor) and the sky color with it.
        m_timeOfDay = std::fmod(m_timeOfDay + dt.asSeconds() / DAY_LENGTH_SECONDS, 1.f);
        // Warp the phase so the daylight half of the sine fills the day and the night is compressed.
        float dayShare = 1.f - NIGHT_FRACTION;
        float phase = m_timeOfDay < dayShare
            ? m_timeOfDay / dayShare * 0.5f
            : 0.5f + (m_timeOfDay - dayShare) / NIGHT_FRACTION * 0.5f;
        float sun = std::sin(phase * 2 * glm::pi<float>()); // +1 at noon, 0 at dawn/dusk, negative through the night
        float daylight = clamp(sun, 0.f, 1.f);
        m_dayFactor = NIGHT_LIGHT + (1.f - NIGHT_LIGHT) * daylight;
        m_skyColor = Window::clearColor * (NIGHT_SKY + (1.f - NIGHT_SKY) * daylight);
    }
    float getTimeOfDay() const { return m_timeOfDay; }
    float getDayFactor() const { return m_dayFactor; }
    vec3 getSkyColor() const { return m_skyColor; }

private:
	static constexpr float DAY_LENGTH_SECONDS = 120.f;
	static constexpr float NIGHT_FRACTION = 0.3f;
	static constexpr float NIGHT_LIGHT = 0.12f;
	static constexpr float NIGHT_SKY = 0.08f;

	// Time of day in [0, 1): 0 sunrise, 0.25 noon, 0.5 sunset, 0.75 midnight. Starts at noon.
	float m_timeOfDay = 0.25f;
    float m_dayFactor = 1.f; // multiplier for terrain light, dimmed at night
	vec3 m_skyColor{ Window::clearColor }; // base sky color dimmed by the current daylight
};