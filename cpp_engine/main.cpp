#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm>

using namespace std;

string escapeJson(const string& s) {
    string out;
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }
    return out;
}
string quote(const string& s) { return "\"" + escapeJson(s) + "\""; }

struct Lesson {
    string id, title, category, summary, content, exercise;
};

vector<Lesson> lessons = {
 {"aperture","Aperture","Exposure","Control light and depth of field.",
  "Aperture is the adjustable opening inside a lens that lets light reach the camera sensor. It is described using f-numbers such as f/1.8, f/4 and f/11. A smaller f-number means a wider opening, which generally lets in more light and can create a shallower depth of field. A larger f-number means a narrower opening, which lets in less light and can keep more of the scene in focus. The best choice depends on available light and how much background blur you want.",
  "Photograph the same subject at a wide aperture and a narrow aperture. Compare the background."},
 {"shutter-speed","Shutter Speed","Exposure","Control motion and the duration of exposure.",
  "Shutter speed is the length of time the sensor is exposed to light. It is commonly measured in fractions of a second, such as 1/1000 s, or in whole seconds. Fast shutter speeds help freeze movement; slower shutter speeds can show motion blur and may require a tripod. If the shutter is too slow for handheld shooting, camera shake can soften the image. Choose a speed based on subject movement, available light and your creative goal.",
  "Photograph moving water or a moving person using two different shutter speeds."},
 {"iso","ISO","Exposure","Understand sensor sensitivity settings and image noise.",
  "ISO describes the camera's sensitivity setting for the captured signal. Raising ISO can help you use a faster shutter speed or a smaller aperture in low light, but it often increases visible noise and can reduce image quality. Use the lowest ISO that still allows the shutter speed and aperture you need. Modern cameras vary, so test your own camera rather than assuming one ISO limit works for every model.",
  "Photograph a still subject at low and high ISO, then compare noise in the shadows."},
 {"exposure-triangle","Exposure Triangle","Exposure","Balance aperture, shutter speed and ISO.",
  "The exposure triangle describes the relationship between aperture, shutter speed and ISO. Aperture affects light and depth of field, shutter speed affects light and motion, and ISO affects the brightness of the recorded signal and image noise. Changing one setting may require changing another to maintain similar brightness. These settings also change the look of the photograph, so exposure is a creative decision rather than brightness alone.",
  "Choose a portrait goal, then decide which setting you would prioritise and why."},
 {"composition","Composition","Composition","Arrange elements to guide the viewer's attention.",
  "Composition is the arrangement of subjects, lines, shapes, colours and empty space inside the frame. Strong composition helps viewers understand what matters in a photograph. The rule of thirds is one useful starting point, but it is not compulsory. Look at the frame edges, remove distractions, consider foreground and background, and choose a viewpoint that supports the story you want to tell.",
  "Take three photographs of one subject from different viewpoints and compare the results."},
 {"focus","Focus","Camera Skills","Make the important subject appear sharp.",
  "Focus determines which distance or subject appears sharpest. Autofocus systems can select a point automatically or let you choose where to focus. Single-shot autofocus suits subjects that remain still, while continuous autofocus is useful for moving subjects. For portraits, focus carefully on the eye closest to the camera when appropriate. Check focus by reviewing the image at a larger size, especially when using a shallow depth of field.",
  "Take a portrait and check whether the eye you intended to emphasise is sharp."},
 {"lighting","Lighting","Light & Colour","Use the direction, quality and colour of light.",
  "Lighting shapes texture, mood, contrast and colour. Soft light has gradual shadows and can be flattering for portraits; hard light creates sharper shadow edges and can look dramatic. Light may come from the front, side or behind the subject, and each direction changes shape and separation. Observe the light before changing camera settings. A window, shade, reflector or simple change of position can improve a photograph without extra equipment.",
  "Photograph the same subject in direct sun and open shade; compare the shadows."},
 {"white-balance","White Balance","Light & Colour","Keep colour casts intentional and believable.",
  "White balance adjusts how the camera interprets colour under different light sources. Daylight, shade, tungsten and fluorescent lighting can produce different colour casts. Auto white balance is convenient, while a preset or custom white balance can provide more consistent results. RAW files generally offer more flexibility for correcting white balance later. Warm or cool colour can also be used intentionally to create a mood.",
  "Photograph a neutral object under two different light sources and compare colour."},
 {"focal-length","Focal Length","Lenses","Understand angle of view and lens choice.",
  "Focal length, measured in millimetres, influences a lens's angle of view. Shorter focal lengths usually show a wider scene, while longer focal lengths show a narrower view. Perspective is primarily determined by camera position; changing lenses and moving to reframe can change the perspective you record. Choose a focal length based on the subject, available space and the visual effect you want.",
  "Photograph a subject with two focal lengths, keeping notes about your camera position."},
 {"depth-of-field","Depth of Field","Lenses","Control how much of the scene appears acceptably sharp.",
  "Depth of field is the range of distances that appear acceptably sharp in a photograph. It is influenced by aperture, focusing distance, focal length and the final viewing conditions. A wide aperture often creates a shallow depth of field, useful for separating a portrait from its background. A narrow aperture can keep more of a landscape sharp, although very small apertures can reduce fine detail through diffraction.",
  "Photograph a close subject with a blurred background, then try a deeper-focus scene."},
 {"hdr","HDR Photography","Advanced","Capture or combine a wider range of brightness.",
  "High Dynamic Range photography aims to represent scenes with a large difference between bright highlights and dark shadows. Some cameras and phones combine multiple exposures or process a single capture to preserve detail. HDR can help in high-contrast scenes, but aggressive processing may create halos, unnatural colours or a flat look. Use it when the scene needs it and review the result for believable tones.",
  "Photograph a scene with a bright window and a darker room; compare HDR options if available."},
 {"raw-jpeg","RAW and JPEG","Workflow","Choose an image format for your workflow.",
  "RAW files preserve more of the sensor data for later editing and often provide greater flexibility for recovering highlights, adjusting white balance and changing tones. JPEG files are processed in-camera, smaller and ready to share, but usually leave less room for major edits. RAW files need compatible software and more storage. The right choice depends on your workflow, storage and how much editing you expect to do.",
  "Compare a RAW and JPEG version of the same scene if your camera supports both."},
 {"golden-hour","Golden Hour","Light & Colour","Use low, warm sunlight creatively.",
  "Golden hour refers to the period around sunrise or sunset when sunlight is often lower and warmer in colour. It can create long shadows, gentle directional light and a warm atmosphere. Conditions vary with weather, season and location. Expose for the important subject, watch for flare and avoid assuming golden hour is always the best light; overcast and midday light can also suit a subject.",
  "Make one photograph with the light behind the subject and one with side light."},
 {"negative-space","Negative Space","Composition","Use uncluttered areas to emphasise a subject.",
  "Negative space is the relatively empty area around the main subject. It can make a composition feel calm, minimal, isolated or spacious and can help the subject stand out. Empty space does not need to be completely blank; a smooth sky or softly blurred background may work. Give negative space a purpose and check that it does not contain distracting bright spots or accidental tangencies.",
  "Place a small subject against a simple background and try two different amounts of empty space."},
 {"metering","Metering and Exposure","Exposure","Read light and avoid unwanted clipping.",
  "Metering estimates scene brightness to help the camera choose or recommend exposure. Evaluative or matrix metering considers much of the frame, centre-weighted metering prioritises the centre, and spot metering reads a small area. Bright snow or a dark stage can fool automatic exposure because the camera does not know your intent. Use exposure compensation and review the histogram when the result does not match your goal." ,
  "Photograph a bright subject against a dark background and compare exposure compensation settings."},
 {"golden-ratio","Golden Ratio and Spiral","Composition","Use a curved visual path to organise a frame.",
  "The golden ratio and golden spiral are compositional guides inspired by a mathematical proportion. A spiral can help you think about how a viewer's eye travels through a photograph toward a focal point. It is not a guarantee of a good image, and real scenes rarely fit a perfect template. Use it as a planning aid when curves, gestures or layered shapes naturally guide attention.",
  "Find a curved path, shoreline or arrangement that leads the eye toward your subject."},
 {"symmetry","Symmetry and Balance","Composition","Create order or intentional visual tension.",
  "Symmetry occurs when visual elements mirror or repeat across an axis. It can make architecture, reflections and centred portraits feel orderly. Asymmetrical balance uses different elements with comparable visual weight instead of mirroring them. Balance can come from size, brightness, colour, detail or subject importance. Neither approach is automatically better; choose the one that supports the image's mood.",
  "Make one symmetrical photograph and another balanced with unequal elements."},
 {"leading-lines","Leading Lines","Composition","Guide attention with lines in the scene.",
  "Leading lines are visible paths such as roads, fences, shorelines, shadows or architectural edges that guide attention. They can lead toward the subject, create depth or pull the viewer through the frame. Lines do not have to meet at the centre. Check where they begin and end, and avoid lines that unintentionally lead the eye out of the image.",
  "Find a path or railing that directs attention toward a clear subject."},
 {"natural-framing","Natural Framing","Composition","Use elements around a subject to focus attention.",
  "Natural framing places a subject inside or behind elements such as a doorway, arch, window or branches. A frame can add depth and context, and it can reduce distractions around the subject. Make sure the framing elements do not overwhelm the subject or create awkward intersections with the edges of the frame.",
  "Photograph a person or object through a doorway or between foreground objects."},
 {"patterns","Patterns and Repetition","Composition","Use repetition, rhythm and interruption.",
  "Repeated shapes, lines, colours or textures create patterns and rhythm. A repeated pattern can feel orderly, while a break in the pattern can become a strong focal point. Look for repetition in buildings, plants, markets or everyday objects. Consider how your viewpoint and framing make the pattern clearer.",
  "Photograph a repeated pattern, then include one element that interrupts it."},
 {"contrast","Contrast and Juxtaposition","Composition","Place visual differences together meaningfully.",
  "Contrast can involve light and dark, warm and cool colour, rough and smooth texture, large and small forms, or old and new subjects. Juxtaposition places elements close enough for viewers to notice a relationship between them. Strong contrast attracts attention, so make sure it supports the story rather than distracting from it.",
  "Find two contrasting elements and compose them so their relationship is clear."},
 {"layers","Foreground, Middle Ground and Background","Composition","Build depth with visual layers.",
  "Foreground, middle ground and background are layers at different distances from the camera. Including useful elements in more than one layer can create depth and help the viewer enter the scene. Layers can be separated by focus, light, colour or overlap. Avoid including foreground clutter that blocks or competes with the subject.",
  "Photograph a scene with a foreground detail, a main subject and a background."},
 {"viewpoint","Perspective and Viewpoint","Composition","Change the camera position to change the story.",
  "Viewpoint includes camera height, angle and position relative to the subject. A low angle can make a subject appear imposing, while a high angle can reveal patterns or context. Moving closer or farther also changes spatial relationships. Try several positions before taking the final image rather than relying only on zoom.",
  "Photograph the same object from eye level, above and below."},
 {"focal-point","Focal Point and Visual Hierarchy","Composition","Make the most important element clear.",
  "A focal point is the element that attracts attention first. Size, brightness, contrast, colour, sharpness and placement can all create visual hierarchy. If several elements compete equally, the viewer may not know where to look. Decide what matters most and use framing, light or simplification to support it.",
  "Make one subject stand out from a busy scene using position or light."},
 {"triangles","Triangles and Geometric Shapes","Composition","Use shapes to create structure.",
  "Triangles, circles, rectangles and diagonals can organise a composition. Triangles formed by people, buildings or objects can create stability or movement depending on their orientation. Look for shapes already present in a scene and use camera position to make them clearer. These are guides, not strict formulas.",
  "Arrange three objects in a triangle and photograph them from different angles."},
 {"look-room","Look Room and Lead Room","Composition","Leave space in the direction of gaze or movement.",
  "Look room is space in front of a person who is looking in a direction. Lead room is space in front of a moving subject. This space can make the image feel open and suggest what may happen next. Tight framing can also work when you want tension or intimacy, so make the choice intentionally.",
  "Photograph a person looking left and right, leaving space in the direction of the gaze."},
 {"scale","Scale and Proportion","Composition","Show size and relationships between objects.",
  "Scale is easier to understand when the frame includes a familiar object or person beside something large or small. Proportion describes the relative sizes of elements. Use these relationships to communicate the size of a building, landscape or object, while remembering that perspective can exaggerate apparent size.",
  "Include a person or familiar object to show the scale of a large scene."},
 {"colour","Colour Theory and Harmony","Composition","Use colour relationships to guide mood and attention.",
  "Colour can unify a photograph or create contrast. Complementary colours sit opposite each other on a colour wheel; analogous colours sit near each other; monochromatic palettes use variations of one hue. Warm and cool colours can also create different impressions. Consider the colours in the whole frame, including distracting bright colours near the edges.",
  "Make one photograph with related colours and another with contrasting colours."},
 {"light-shadow","Light and Shadow as Composition","Composition","Treat bright and dark areas as shapes.",
  "Light and shadow can guide the eye, create silhouettes, reveal texture and simplify a scene. Shadows can become leading lines or repeating patterns. Watch bright areas because viewers often look at them first. Use exposure and camera position to preserve the important parts of the scene.",
  "Photograph a scene where a shadow leads toward the subject."},
 {"simplicity","Simplicity and Clutter Control","Composition","Remove distractions and strengthen the subject.",
  "Simplicity means making the intended subject easy to understand, not necessarily making the scene empty. Check frame edges, bright spots, overlapping objects and backgrounds. Move your position or change the framing to remove distractions. Sometimes a busy frame is useful when the complexity is part of the story.",
  "Take one photograph as usual, then simplify it by changing position or cropping."},
 {"lines","Horizontal, Vertical and Diagonal Lines","Composition","Use line direction to influence visual energy.",
  "Horizontal lines often suggest calm or stability, vertical lines can emphasise height, and diagonals can add energy or movement. These associations are not universal, but they are useful ways to analyse a frame. Keep important architectural lines intentional and watch for unwanted tilts.",
  "Photograph horizontal, vertical and diagonal lines and compare their visual feel."},
 {"horizon","Horizon Placement","Composition","Give the sky or land the space it deserves.",
  "Horizon placement changes the balance between sky and land. A low horizon can emphasise dramatic clouds; a high horizon can emphasise foreground texture. A centred horizon can work well for reflections or symmetry. Keep the horizon level unless a tilt is intentional.",
  "Make three versions of the same landscape with different horizon positions."},
 {"separation","Subject Separation and Depth of Field","Composition","Make the subject distinct from its surroundings.",
  "Subject separation means helping the main subject stand apart from the background. You can use focus, aperture, distance, light, colour or tonal contrast. Background blur is one option, not a requirement. Check that objects do not appear to grow out of the subject's head or merge confusingly with its outline.",
  "Move your subject away from the background and compare the result."},
 {"decisive-moment","Timing and the Decisive Moment","Composition","Capture gestures and relationships at the right instant.",
  "Timing matters when expressions, movement or relationships between elements change quickly. Anticipate what may happen, prepare your framing and take several frames when appropriate. The decisive moment is not limited to street photography; it can matter in sports, wildlife, portraits and everyday scenes.",
  "Photograph a moving subject or gesture and choose the frame with the clearest moment."},
 {"aspect-ratio","Aspect Ratio and Cropping","Composition","Choose framing that suits the image's purpose.",
  "Aspect ratio is the relationship between image width and height. Horizontal frames can suit landscapes, vertical frames can emphasise height or portraits, and square frames can create compact balance. Cropping can strengthen a composition, but leave enough room for important context and intended display sizes.",
  "Photograph a subject vertically and horizontally, then compare the storytelling."}
};

struct Question { string id, question, a, b, c, d, answer, explanation; };
vector<Question> questions = {
 {"q1","Which aperture generally creates a shallower depth of field?","f/16","f/11","f/1.8","f/8","C","A wider aperture such as f/1.8 generally creates a shallower depth of field, depending on distance and focal length."},
 {"q2","Which setting most directly controls how long motion is recorded?","ISO","Shutter speed","White balance","Focal length","B","Shutter speed determines the exposure duration and strongly affects motion blur."},
 {"q3","Where are the main guide lines in the rule of thirds?","At the frame edges only","A 2 by 2 grid","Two horizontal and two vertical lines dividing the frame into thirds","A spiral only","C","The rule of thirds divides the frame into a 3 by 3 grid."},
 {"q4","What does negative space mean?","The darkest pixel","Uncluttered space around a subject","An underexposed photograph","A camera setting","B","Negative space is relatively empty or simple area that helps emphasise a subject."},
 {"q5","Which is often useful for freezing fast action?","A very slow shutter speed","A fast shutter speed","A warmer white balance","A narrower crop only","B","A fast shutter speed reduces the time during which movement is recorded."},
 {"q6","Which composition technique uses a road or fence to guide attention?","Leading lines","White balance","Exposure compensation","Depth of field","A","Leading lines guide the viewer through the frame or toward a subject."},
 {"q7","What is white balance used for?","Changing the camera's physical lens","Adjusting colour interpretation under different light","Setting shutter duration","Removing all shadows","B","White balance helps correct or intentionally adjust colour casts from different light sources."},
 {"q8","Which statement about composition rules is best?","Every photo must use all rules","Rules cannot be broken","They are useful guides chosen to support the image","They replace subject choice","C","Composition principles are tools, not compulsory formulas."},
 {"q9","What can a higher ISO setting often introduce?","More visible image noise","A wider physical lens opening","A different focal length","A guaranteed sharper image","A","Higher ISO can increase visible noise, though the exact result depends on the camera and exposure."},
 {"q10","What does focal length mainly influence in a lens?","The lens's angle of view","The battery capacity","The date stored in metadata","The colour of the camera body","A","Focal length is a key factor in the angle of view."}
};

void printLessons() {
    cout << "[";
    for (size_t i = 0; i < lessons.size(); ++i) {
        const auto& l = lessons[i];
        if (i) cout << ",";
        cout << "{\"id\":" << quote(l.id)
             << ",\"title\":" << quote(l.title)
             << ",\"category\":" << quote(l.category)
             << ",\"summary\":" << quote(l.summary)
             << ",\"content\":" << quote(l.content)
             << ",\"exercise\":" << quote(l.exercise) << "}";
    }
    cout << "]";
}
void printQuestions() {
    cout << "[";
    for (size_t i = 0; i < questions.size(); ++i) {
        const auto& q = questions[i];
        if (i) cout << ",";
        cout << "{\"id\":" << quote(q.id) << ",\"question\":" << quote(q.question)
             << ",\"options\":["
             << "{\"value\":\"A\",\"label\":" << quote(q.a) << "},"
             << "{\"value\":\"B\",\"label\":" << quote(q.b) << "},"
             << "{\"value\":\"C\",\"label\":" << quote(q.c) << "},"
             << "{\"value\":\"D\",\"label\":" << quote(q.d) << "}]}";
    }
    cout << "]";
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "{\"error\":\"Missing command\"}";
        return 0;
    }
    string cmd = argv[1];
    if (cmd == "lessons") { printLessons(); return 0; }
    if (cmd == "questions") { printQuestions(); return 0; }
    if (cmd == "grade" && argc >= 4) {
        string id = argv[2], answer = argv[3];
        for (const auto& q : questions) {
            if (q.id == id) {
                bool correct = (answer == q.answer);
                cout << "{\"correct\":" << (correct ? "true" : "false")
                     << ",\"correctAnswer\":" << quote(q.answer)
                     << ",\"explanation\":" << quote(q.explanation) << "}";
                return 0;
            }
        }
        cout << "{\"error\":\"Question not found\"}";
        return 0;
    }
    if (cmd == "simulate" && argc >= 5) {
        double aperture = atof(argv[2]);
        double shutter = atof(argv[3]);
        double iso = atof(argv[4]);
        if (aperture <= 0 || shutter <= 0 || iso <= 0) {
            cout << "{\"error\":\"Aperture, shutter speed and ISO must be positive.\"}";
            return 0;
        }
        double ev100 = log2((aperture * aperture) / shutter) - log2(iso / 100.0);
        double brightness = max(0.0, min(100.0, 50.0 + (12.0 - ev100) * 7.0));
        cout << "{\"ev100\":" << ev100 << ",\"brightness\":" << brightness
             << ",\"note\":\"Educational estimate only; actual exposure depends on scene lighting, metering and camera characteristics.\"}";
        return 0;
    }
    if (cmd == "advisor" && argc >= 5) {
        string subject = argv[2], light = argv[3], effect = argv[4];
        string aperture = "f/5.6", shutter = "1/250 s", iso = "ISO 200";
        string reason = "A balanced starting point; adjust after reviewing the exposure.";
        if (subject == "portrait") {
            aperture = (effect == "blur") ? "f/1.8–f/2.8" : "f/4–f/5.6";
            shutter = "1/200 s or faster";
            reason = "A wider aperture can soften the background; keep shutter speed high enough for natural movement and focus carefully on the eye.";
        } else if (subject == "landscape") {
            aperture = "f/8–f/11";
            shutter = "Use tripod if shutter is slow";
            reason = "A moderate aperture often balances depth of field and detail. Focus carefully and watch highlights.";
        } else if (subject == "action") {
            aperture = "As wide as needed for light";
            shutter = "1/1000 s or faster";
            iso = "Raise ISO as needed";
            reason = "A fast shutter speed helps freeze movement; use continuous autofocus if available.";
        } else if (subject == "night") {
            aperture = "Wide aperture if appropriate";
            shutter = "Tripod for long exposures";
            iso = "ISO 800–3200 as a starting range";
            reason = "Low light requires a trade-off between motion, noise and depth of field. Stabilise the camera for long exposures.";
        } else if (subject == "street") {
            aperture = "f/4–f/8";
            shutter = "1/250 s or faster";
            reason = "A moderately fast shutter helps capture spontaneous moments. Be respectful of people's privacy and local rules.";
        } else if (subject == "macro") {
            aperture = "f/5.6–f/11";
            shutter = "Use stable support";
            reason = "Close focusing produces shallow depth of field; consider extra light and careful focus placement.";
        }
        if (light == "low") iso = "ISO 800–3200 as needed";
        if (light == "bright") iso = "ISO 100–200";
        if (light == "moving") shutter = "1/500 s or faster";
        cout << "{\"aperture\":" << quote(aperture) << ",\"shutter\":" << quote(shutter)
             << ",\"iso\":" << quote(iso) << ",\"reason\":" << quote(reason)
             << ",\"warning\":\"These are starting suggestions, not universal settings. Check your camera meter, focus and histogram.\"}";
        return 0;
    }
    cout << "{\"error\":\"Unknown command or missing arguments\"}";
    return 0;
}
